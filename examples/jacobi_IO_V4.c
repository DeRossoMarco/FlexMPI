/**
* @version		FlexMPI v1.4
* @copyright	Copyright (C) 2017 Universidad Carlos III de Madrid. All rights reserved.
* @license		GNU/GPL, see LICENSE.txt
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You have received a copy of the GNU General Public License in LICENSE.txt
* also available in <http://www.gnu.org/licenses/gpl.html>.
*
* See COPYRIGHT.txt for copyright notices and details.
*/


/****************************************************************************************************************************************
*
*	Jacobi iterative method
*
****************************************************************************************************************************************/
#include <stdio.h>
#include <mpi.h> 
#include <empi.h>
#include <math.h>
#include "papi.h"

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/time.h>
#include <errno.h>
#include "error.h"


// Delete: for compatibility with FlexMPI I/O
MPI_File fh;

//function prototype

void jacobi (int dim, int *rank, int *size, double *A, double *b, double *x, double *x_old, int it, int itmax, double diff_limit, int cpu_i, int com_i, int io_i, int appd_id, char *argv[]);
void load_matrix (int dim, int despl, int count, double *A, double *b);
void generate_matrix (int dim, int count, double *A, double *b);

double energy1 = 0;
double energy2 = 0;
double power1 = 0;
double power2 = 0;

double tcomm_r = 0, tcomp_r = 0;



void diep(char *s)
{
  perror(s);
  exit(1);
}

//main
int main (int argc, char *argv[])
{
	int dim, rank, size, itmax, type, despl, count, it = 0;
	int cpu_i,com_i,io_i,app_id;

	double *A = NULL, *b = NULL, *x = NULL, *x_old = NULL, comp_ini, comp_fin, ldata_ini, ldata_fin;

	double diff_tol = 0.0;

	char mpi_name[128];

	int len;

    

	// MPI init
	MPI_Init (&argc, &argv);
	
	//Get rank & size

	MPI_Comm_rank (EMPI_COMM_WORLD, &rank);
	MPI_Comm_size (EMPI_COMM_WORLD, &size);
	MPI_Get_processor_name (mpi_name, &len);


    
	if (argc < 8) {

        printf ("Jacobi usage: ./jacobi <dim> <itmax> <diff_tol> <cpu_intensity> <com_intensity> <IO_intensity> <nping_server_name>\n");
		MPI_Abort (MPI_COMM_WORLD,1);
	}

	//get dim
	dim = atoi (argv[1]);

	//get itmax
	itmax = atoi (argv[2]);

	//get diff tolerance
	diff_tol = atof (argv[3]);

	//get CPU intensity
	cpu_i = atoi(argv[4]);
	if(cpu_i<1) cpu_i=1;
	
	//get communication intensity
	com_i = atoi(argv[5]);
	//if(com_i<1) com_i=1;

	//get IO intensity
	io_i = atoi(argv[6]);

	//application id
	app_id = atoi(argv[7]);

	// argv[8] is the nping server name -> Used in Jacobi function

	b = (double*) calloc (dim, sizeof (double));
	x = (double*) calloc (dim, sizeof (double));
	x_old = (double*) calloc (dim, sizeof (double));

	//get worksize
	EMPI_Get_wsize (rank, size, dim, &despl, &count, NULL, NULL);

	A = (double*) calloc (count * dim, sizeof (double));

	//register dense matrix 
	EMPI_Register_dense ("matrix", A, MPI_DOUBLE, dim, EMPI_DISJOINT);
	//register vector
	EMPI_Register_vector ("x_old", x_old, MPI_DOUBLE, dim, EMPI_SHARED);

	EMPI_Get_type (&type);

	if (type == EMPI_NATIVE) {

		MPI_Barrier (EMPI_COMM_WORLD);

		ldata_ini = MPI_Wtime ();

		//load matrix
		//load_matrix (dim, despl, count, A, b);
		generate_matrix (dim, count, A, b);
		
		ldata_fin = MPI_Wtime ();

		printf ("[%i] Process spawned in %s | Data loaded in %lf secs.\n", rank, mpi_name, ldata_fin-ldata_ini);

		MPI_Barrier (EMPI_COMM_WORLD);

	} else {

		ldata_ini = MPI_Wtime ();

		//get shared array
		EMPI_Get_shared (&it);

		ldata_fin = MPI_Wtime ();

		printf (" [%i] Process spawned in %s at %i | Data received in %lf secs.\n", rank, mpi_name, it, ldata_fin-ldata_ini);
	}



	comp_ini = MPI_Wtime ();

	printf (" [%d] Jacobi started \n", rank);
	if(rank==0) printf(" [%d] Configuration: \t nprocs: %d \t dim: %d \t itmax: %d \t diff_tol: %f \t cpu_intensity: %d  \t com_intensity: %d \t IO_intensity: %d \n\n",rank,size,dim,itmax,diff_tol,cpu_i,com_i,io_i);

	//jacobi
	jacobi (dim, &rank, &size, A, b, x, x_old, it, itmax, diff_tol, cpu_i, com_i, io_i, app_id, argv);

	comp_fin = MPI_Wtime ();

	if (rank == 0) printf (" [%i] Jacobi finished in %lf seconds - cost %f\n", rank, comp_fin-comp_ini, EMPI_GLOBAL_cum_cost);

	
	//free
	EMPI_free (A, "matrix");
	free (b);
	free (x);
	EMPI_free (x_old, "x_old");



	if (rank == 0) printf (" Terminating MPI \n");	
	//FLEXMPI finalize
	MPI_Finalize();


	exit(1);
}

//parallel jacobi
void jacobi (int dim, int *rank, int *size, double *A, double *b, double *x, double *x_old, int it, int itmax, double diff_limit, int cpu_i, int com_i, int io_i, int app_id, char *argv[])
{
	double tcomm = 0, tcomp = 0;
	
	double diff = 0, axi = 0, *x_new = NULL;
	double t1,t2,cnt_io0=0,cnt_io1=0;
	int i,n, m, k, desp, count, status, *displs = NULL, *vcounts = NULL, type;
	
	char bin[1024],npingcmd[1024]; 
	sprintf(bin,"%s",argv[0]);
	
    char file[200];
	
	sprintf(file,"datafileN1_%d.out",app_id);

		
	EMPI_Get_type (&type);
	
	//malloc vcounts and displs array
	displs = (int*) malloc (*size * sizeof(int));
	vcounts = (int*) malloc (*size * sizeof(int));
	
	x_new = (double*) calloc (dim, sizeof (double));

	//get worksize
	EMPI_Get_wsize (*rank, *size, dim, &desp, &count, vcounts, displs);
	
	t1=MPI_Wtime();
	for (; it < itmax; it ++) {
		
		//monitor init
		EMPI_Monitor_init ();
        
		for(k=0;k<cpu_i;k++){
			//matrix rows
			for (n = 0; n < count; n ++) {

				axi = 0;

				//matrix cols
				for (m = 0; m < dim; m ++) { 

					if ((n+desp) != m) axi += (A[(n*dim)+m] * x_old[m]);
				}

				x[n+desp] = (( b[n+desp] - axi ) / A[(n*dim)+(n+desp)]);
			}
		}
		
		//Allgather x vector
		for(i=0;i<com_i;i++) 	MPI_Allgatherv (x+desp, count, MPI_DOUBLE, x_new, vcounts, displs, MPI_DOUBLE, EMPI_COMM_WORLD);

		for (diff = 0, n = 0; n < dim; n ++) diff += fabs (x_new[n] - x_old[n]);

		//memory copy
		memcpy (x_old, x_new, (dim*sizeof(double)));

       
		//monitor end
		EMPI_Monitor_end (rank, size, it, itmax, &count, &desp, &vcounts, &displs, NULL, argv+1, bin);

		//get new array address
		A = EMPI_Get_addr("matrix");
		x_old = EMPI_Get_addr("x_old");

		EMPI_Get_status (&status);

		//printf("Process %d finished iteration %d of %d\n", *rank, it, itmax);
		if (status == EMPI_REMOVED) break;

		
	}

	t2=MPI_Wtime(); 

	//get aggregated tcomp
	EMPI_Get_aggregated_tcomp (&tcomp);
	EMPI_Get_aggregated_tcomm (&tcomm);
	tcomp_r = tcomp;
	tcomm_r = tcomm;
	
	printf ("[%i] Jacobi finished in %i iterations, %f diff value - tcomp %lf tcomm %lf - overhead %lf lbalance %lf rdata %lf processes %lf reconfiguring %lf other %lf\n", *rank, it,diff, tcomp, tcomm, EMPI_GLOBAL_tover, EMPI_GLOBAL_overhead_lbalance, EMPI_GLOBAL_overhead_rdata, EMPI_GLOBAL_overhead_processes, EMPI_GLOBAL_overhead_rpolicy, (EMPI_GLOBAL_tover - EMPI_GLOBAL_overhead_lbalance - EMPI_GLOBAL_overhead_processes - EMPI_GLOBAL_overhead_rdata - EMPI_GLOBAL_overhead_rpolicy));
	printf("\n [%i] Total execution time: %f \t io1 %f \t io2 %f \n",*rank,t2-t1,cnt_io0,cnt_io1);
	
	if(*rank==0){
		sprintf(npingcmd,"nping --udp -g %d -p %d  -c 1 %s --data-string \"Aplication terminated\">/dev/null",EMPI_GLOBAL_recvport+200,EMPI_GLOBAL_sendport,argv[8]);
		system(npingcmd);
	}
	sleep(5);

	free (displs);
	free (vcounts);

	free (x_new);
}

//load matrix
void load_matrix (int dim, int despl, int count, double *A, double *b) {

	int n = 0, m = 0, size_double = 4, size_char = 1;
	int err;
	FILE *flDense = NULL;

	flDense = fopen ("./matrices/densematrix.dat", "r");

	for (n = 0; n < dim; n ++) b[n] = (n % 5) + 1;

		fseek (flDense, (despl * dim * (size_double + size_char)), SEEK_SET);

	for (n = 0; n < count; n ++){
	
        for (m = 0; m < dim; m ++){
		
			err = fscanf (flDense, "%lf\n", &A[(n*dim)+m]);
			if(err == EOF){
				perror("Error fscanf\n");
			}
        }
		fclose (flDense);
    }
}


// Generate matrix's values
void generate_matrix (int dim, int count, double *A, double *b) {

	int n = 0, m = 0;

	for (n = 0; n < dim; n ++) b[n] = (n % 5) + 1;

	for (n = 0; n < count; n ++){
	
        for (m = 0; m < dim; m ++){
		
			//A[(n*dim)+m]= 30-60*(rand() / RAND_MAX ) ;  
			A[(n*dim)+m]=(double)((n*dim)+m);
		}
	}
}




