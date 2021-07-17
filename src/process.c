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
 *																																		*
 *	FLEX-MPI																															*
 *																																		*
 *	File:       process.c																													*
 *																																		*
 *																																		*
 ****************************************************************************************************************************************/

/* include */
#include <empi.h>

/* headers */

/****************************************************************************************************************************************
*
*	'EMPI_Spawn_process'
*
****************************************************************************************************************************************/
static int EMPI_Spawn_process (char *argv[], char *bin, MPI_Info info, int nprocs);

/****************************************************************************************************************************************
*
*	'EMPI_Remove_process'
*
****************************************************************************************************************************************/
static int EMPI_Remove_process (int rank);

/* implementation */

/****************************************************************************************************************************************
*
*	'EMPI_Spawn_process'
*
****************************************************************************************************************************************/
static int EMPI_Spawn_process (char *argv[], char *bin, MPI_Info info, int nprocs) {

	//debug
	#if (EMPI_DBGMODE > EMPI_DBG_QUIET)
		fprintf (stdout, "\n*** DEBUG_MSG::enter::EMPI_Spawn_process in <%s> ***\n", __FILE__);
	#endif

	int *mpierr, err;

	MPI_Comm childcomm;
	MPI_Comm dupcomm;
	
	mpierr=(int*)malloc(nprocs*sizeof(int));
	if(mpierr==NULL) {fprintf (stderr, "Error in EMPI_Spawn_process: not enough memory\n"); return -1;}
	
	err = MPI_Comm_spawn (bin, argv, nprocs, info, EMPI_root, EMPI_COMM_WORLD, &childcomm, mpierr);
	if (err) {fprintf (stderr, "Error in MPI_Comm_spawn\n"); return err;}

	//Merge intercommunicator
	err = MPI_Intercomm_merge (childcomm, 0, &EMPI_COMM_WORLD);
	if (err) {fprintf (stderr, "Error in MPI_Intercomm_merge\n"); return err;}
	
   	//Disconnect aux communicator
	err = MPI_Comm_disconnect (&childcomm);
	if (err) {fprintf (stderr, "Error in MPI_Comm_disconnect\n"); return err;}

	//debug
	#if (EMPI_DBGMODE > EMPI_DBG_QUIET)
		fprintf (stdout, "\n*** DEBUG_MSG::exit::EMPI_Spawn_process in <%s> ***\n", __FILE__);
	#endif

	free(mpierr);
	return err;
}

/****************************************************************************************************************************************
*
*	'EMPI_Spawn'
*
****************************************************************************************************************************************/
int EMPI_Spawn (int nprocs, char *argv[], char *bin, int *hostid, MPI_Info *info) {

	//TODO: volver a permitir MPI_Info

	//debug
	#if (EMPI_DBGMODE > EMPI_DBG_QUIET)
		fprintf (stdout, "\n*** DEBUG_MSG::enter::EMPI_Spawn in <%s> ***\n", __FILE__);
	#endif

	int n, m, rank, size, rprocs, tag=997, sdata[4], err,cnt,last_host;
	int nhost;
	int *nhost_list;
	char *string_hostlist,tmpchar[100];
	EMPI_host_type *hostlist = NULL;
	MPI_Info infoc;

	nhost_list=(int*)malloc(nprocs*sizeof(int));
	string_hostlist=(char *)malloc(nprocs*40*sizeof(char));
	if(string_hostlist==NULL || nhost_list==NULL){
	  	fprintf(stderr,"Error in EMPI_Spawn: not enough memory\n");
	  	exit(1);		
	}
		
	MPI_Comm_rank (EMPI_COMM_WORLD, &rank);
	MPI_Comm_size (EMPI_COMM_WORLD, &size);
  
  	//printf("Process %d in group of %d processes\n", rank, size);
	
	//reset global variables
	EMPI_GLOBAL_concurrency = EMPI_TRUE;
	if (EMPI_GLOBAL_lbalance_disabled == EMPI_FALSE) EMPI_GLOBAL_lbalance = EMPI_TRUE;

	// Only for root: for each new created process generates the destination list
	if (rank == EMPI_root) {

		//Create info 	
		MPI_Info_create (&infoc);
		
		cnt=0;
		last_host=-1;
		
		for (n = 0; n < nprocs; n ++) {
			
			// Nhost is the compute node sellected to spawm the process
			if (hostid != NULL)
				nhost = hostid[n];
			else{
				EMPI_Sched_spawn (&nhost);
			}
				
			
			nhost_list[n]=nhost;			
			hostlist = EMPI_GLOBAL_hostlist;

			for (m = 0; m < EMPI_GLOBAL_nhosts; m ++) {

				// New node is included in the list
				if ((hostlist != NULL)&&(nhost == m)){						
			
					// The new node is different than the previous one
					if(last_host!=m ){
						
						// Updates values of the previous coincidence
						if(last_host!=-1){
							strcat(string_hostlist, ":");		
							sprintf(tmpchar,"%d",cnt);
							strcat(string_hostlist,tmpchar);	
							cnt=0;
						}
						last_host=m;
						
						// Creates the string with the list of hosts
						if(n==0) {
							strcpy(string_hostlist, hostlist->hostname);
						}
						else{
							strcat(string_hostlist, ",");							
							strcat(string_hostlist, hostlist->hostname);
						}							
					}

					
					 // Counts the number of processs in the class
					cnt++;
					//Increases the number of processes of the class
					hostlist->nprocs++;
					
				}					
				else{ 
					hostlist = hostlist->next;
				}
			}
		}
		
		// Adds last values
		strcat(string_hostlist, ":");		
		sprintf(tmpchar,"%d",cnt);
		strcat(string_hostlist,tmpchar);		
		
		printf("Host list: %s \n",string_hostlist);
		MPI_Info_set (infoc, "hosts",string_hostlist);
	}
		
	//Spawn new process
	err = EMPI_Spawn_process (argv, bin, infoc,nprocs);
	
	// Only for root: for each new created process
	if (rank == EMPI_root) {
		MPI_Info_free (&infoc);

		for (n = 0; n < nprocs; n ++) {
		
			rprocs = nprocs - n - 1;

			//set number of remaining spawns, minprocs and maxprocs
			sdata[0] = rprocs;
			sdata[1] = EMPI_GLOBAL_minprocs;
			sdata[2] = (EMPI_GLOBAL_iteration+1); //next iteration
			sdata[3] = nhost_list[n];

			//send number of remaining spawns, minprocs and maxprocs. Size is the comm size before the spawn			
			PMPI_Send (sdata, 4, MPI_INT, size+n, tag, EMPI_COMM_WORLD);
		}
	}

	free(string_hostlist);
	free(nhost_list);
	
	//debug
	#if (EMPI_DBGMODE > EMPI_DBG_QUIET)
		fprintf (stdout, "\n*** DEBUG_MSG::exit::EMPI_Spawn in <%s> ***\n", __FILE__);
	#endif

	return err;
}

/****************************************************************************************************************************************
*
*	'EMPI_Remove_process'
*
****************************************************************************************************************************************/
static int EMPI_Remove_process (int rank) {

	//debug
	#if (EMPI_DBGMODE > EMPI_DBG_QUIET)
		fprintf (stdout, "\n*** DEBUG_MSG::enter::EMPI_Remove_process in <%s> ***\n", __FILE__);
	#endif

	int err, size, myrank,rank2;

	MPI_Comm aux;

	MPI_Group ggroup, exclgroup;
	
	rank2=rank;

	//Get rank & size
	MPI_Comm_rank (EMPI_COMM_WORLD, &myrank);
	MPI_Comm_size (EMPI_COMM_WORLD, &size);
	
	
	if (size > EMPI_GLOBAL_minprocs) {


		//remove
		EMPI_GLOBAL_concurrency = EMPI_TRUE;

		//Create group
		MPI_Comm_group (EMPI_COMM_WORLD, &ggroup);


		//Group of excluded rank
		MPI_Group_excl (ggroup, 1, &rank, &exclgroup);

		//Create new communicator
		err = MPI_Comm_create (EMPI_COMM_WORLD, exclgroup, &aux);
		if (err) {fprintf (stderr, "Error in MPI_Comm_create\n"); return err;}
		


		
		// Disconnect original communicator
		//err = MPI_Comm_disconnect (&EMPI_COMM_WORLD); // Stalls pending of closing other processes
		err = MPI_Comm_free(&EMPI_COMM_WORLD);
		if (err) {fprintf (stderr, "Error in MPI_Comm_disconnect\n"); return err;}
		
		//Group free
		MPI_Group_free (&ggroup);
		MPI_Group_free (&exclgroup);
		
		if (myrank != rank2) {

			//Dupplicate communicator
			err = MPI_Comm_dup (aux, &EMPI_COMM_WORLD);
			if (err) {fprintf (stderr, "Error in MPI_Comm_dup\n"); return err;}

			//Disconnect communicator
			err = MPI_Comm_disconnect (&aux);
			if (err) {fprintf (stderr, "Error in MPI_Comm_disconnect\n"); return err;}

		} else{
			
			//set process status
			EMPI_GLOBAL_status = EMPI_REMOVED;
		}
	} else {

		fprintf (stderr, "Error in EMPI_Remove_process: attempting to remove an initial process\n");

		MPI_Abort (EMPI_COMM_WORLD, -1);
	}

	//debug
	#if (EMPI_DBGMODE > EMPI_DBG_QUIET)
		fprintf (stdout, "\n*** DEBUG_MSG::exit::EMPI_Remove_process in <%s> ***\n", __FILE__);
	#endif

	return err;
}

/****************************************************************************************************************************************
*
*	'EMPI_Remove'
*
****************************************************************************************************************************************/
int EMPI_Remove (int nprocs, int *rank) {

	//debug
	#if (EMPI_DBGMODE > EMPI_DBG_QUIET)
		fprintf (stdout, "\n*** DEBUG_MSG::enter::EMPI_Remove in <%s> ***\n", __FILE__);
	#endif

	//FIXME: cuando un host se queda con 0 nprocs poner sus mflops a -1, para que funcione bien Set_host_perf.

	int n, m, nrank, size, status, err, myrank, nhost, tag = 891;

	MPI_Status mpi_status;

	MPI_Comm_rank (EMPI_COMM_WORLD, &myrank);
	MPI_Comm_size (EMPI_COMM_WORLD, &size);

	EMPI_host_type *hostlist = NULL;
	
	if ((size - nprocs) < EMPI_GLOBAL_minprocs) {

		fprintf (stderr, "\nError in EMPI_Sched_remove: attempting to remove native processes\n");

		MPI_Abort (EMPI_COMM_WORLD, -1);
	}


	if (rank != NULL) {

		for (n = 0; n < nprocs; n ++) {

			if (rank[n] < EMPI_GLOBAL_minprocs) {

				fprintf (stderr, "\nError in EMPI_Remove: attempting to remove a native process\n");

				MPI_Abort (EMPI_COMM_WORLD, -1);
			}

			EMPI_Get_status (&status);

			if (status == EMPI_ACTIVE) {

				if (rank[n] == myrank) PMPI_Send (&EMPI_GLOBAL_hostid, 1, MPI_INT, EMPI_root, tag, EMPI_COMM_WORLD);

				if (myrank == EMPI_root) {

					PMPI_Recv(&nhost, 1, MPI_INT, rank[n], tag, EMPI_COMM_WORLD, &mpi_status);

					//pointer to global variable
					hostlist = EMPI_GLOBAL_hostlist;

					for (m = 0; m < EMPI_GLOBAL_nhosts; m ++) {

						if (nhost == hostlist->id) {

							hostlist->nprocs --;
						}

						hostlist = hostlist->next;
					}
				}

				//Remove process
				err = EMPI_Remove_process (rank[n]);
			}
		}

	} else {

		for (n = 0; n < nprocs; n ++) {

			EMPI_Get_status (&status);
			if (status == EMPI_ACTIVE) {

				//schedule process to being removed
				EMPI_Sched_remove (&nrank, &nhost);

				if (myrank == EMPI_root) {

					//pointer to global variable
					hostlist = EMPI_GLOBAL_hostlist;

					for (m = 0; m < EMPI_GLOBAL_nhosts; m ++) {

						if (nhost == hostlist->id) {

							hostlist->nprocs --;
						}

						hostlist = hostlist->next;
					}
				}

				//Remove process
				
				err = EMPI_Remove_process (nrank);
			}
		}
	}

	hostlist = NULL;

	//debug
	#if (EMPI_DBGMODE > EMPI_DBG_QUIET)
		fprintf (stdout, "\n*** DEBUG_MSG::exit::EMPI_Remove in <%s> ***\n", __FILE__);
	#endif

	return err;
}
