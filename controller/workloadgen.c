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
 FLEX-MPI
 
File:workloadgen.c																											
 ****************************************************************************************************************************************/
 

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <math.h>
#include <errno.h>
#define NCLASES   500

int GLOBAL_DIFFERENTNODES, GLOBAL_EXCLCONTROLLER;
int CONTROLLERPROCS;

// This file generates the workload  
static void Workload_generator (char *filename,char appname[1000][100],int napp, int * nproc, int * Nsize,int * NIO, int * NCPU, int * NCOM, double * IOaction, int * NumIter) 
{
 	char LOCAL_hclasses[NCLASES][128]; //host classes
	char LOCAL_rclasses[NCLASES][128]; //host classes
	int  LOCAL_ncores[NCLASES];
	int  LOCAL_nhclasses,LOCAL_totclasses;
	FILE *file;
	int i,j;
	int reqcor,availcor;
	char controllernode[256],line[2048],tmp[512],readline[1000],token[] = ":", *record = NULL;
    int currclass[1000]; // Current class that is being considered
    


	
	// Parses the node file
	printf("\n ### Reading the nodefile ... \n");
	if ((file = fopen (filename, "r")) == NULL) {
		fprintf (stderr, "\nError opening nodefile in Workload_generator %s \n",filename);
		exit(1);
	}

	printf("\n   Node names: \t name \t \t alias \t \t #cores \n");
	// Read and parse node file that contains the node file names
	LOCAL_nhclasses=0;
    LOCAL_totclasses=0;
	availcor=0;
	while (fscanf (file, "%s\n", readline) != EOF) {
		//Get values
		record = strtok (readline, token);  // Node name
        
		if(record==NULL){fprintf (stderr, "\nError1 parsing Parse_malleability %s\n",filename);exit(1); }
		strncpy(LOCAL_hclasses[LOCAL_nhclasses],record,strlen(record));
		record = strtok (NULL, token);  // Num procs. 
		if(record==NULL){fprintf (stderr, "\nError3 parsing Parse_malleability %s\n",filename);exit(1); }
		LOCAL_ncores[LOCAL_nhclasses]=atoi(record);
        
        if(LOCAL_nhclasses==0 && GLOBAL_EXCLCONTROLLER) CONTROLLERPROCS=LOCAL_ncores[LOCAL_nhclasses]; // Sets exclusive controller node
        
		availcor+=LOCAL_ncores[LOCAL_nhclasses];
		record = strtok (NULL, token);  // Node alias
		if(record==NULL){fprintf (stderr, "\nError2 parsing Parse_malleability %s\n",filename);exit(1); }
		strncpy(LOCAL_rclasses[LOCAL_nhclasses],record,strlen(record));
		
		printf(" \t \t %s \t %s \t %d \n",LOCAL_hclasses[LOCAL_nhclasses],LOCAL_rclasses[LOCAL_nhclasses],LOCAL_ncores[LOCAL_nhclasses]);
		
		LOCAL_nhclasses++;
        LOCAL_totclasses++;
	}
	fclose(file);
	
    
	// Checks if there are enough resources
	reqcor=CONTROLLERPROCS; // The controller an initial number of cores
	
	for(i=0;i<napp;i++){
		reqcor+=nproc[i];
	}
	
	printf("\n   Required cores: %d, existing cores: %d \n\n",reqcor,availcor);
	
	if(reqcor>availcor){
		fprintf (stderr, "\nError in Workload_generator:  not enough resources \n \n");
		exit(1);
	}
	
	LOCAL_nhclasses=0;
	
	// Assigns the controller
	
	// Generates the outputfile
	if ((file = fopen ("controller.dat", "w")) == NULL) {
		fprintf (stderr, "\nError opening nodefile in Workload_generator %s \n",filename);
		exit(1);
	}
	
	
	
	strcpy(controllernode,LOCAL_hclasses[LOCAL_nhclasses]);
	//LOCAL_ncores[LOCAL_nhclasses]--;
	LOCAL_ncores[LOCAL_nhclasses]-=CONTROLLERPROCS;
	printf("\n ### Controller node: \n \t  %s",controllernode);

	fprintf(file,"%s\n",controllernode);
	fclose(file);

	// Generates the outputfile
	if ((file = fopen ("workload.dat", "w")) == NULL) {
		fprintf (stderr, "\nError opening nodefile in Workload_generator %s \n",filename);
		exit(1);
	}

	printf("\n\n ### Controller workfile: ");
	LOCAL_nhclasses=0;
	
    currclass[0]=0;
    if(GLOBAL_DIFFERENTNODES) currclass[1]=LOCAL_totclasses;
    else currclass[1]=0; 
    
	// Creates the nodelist
	for(i=0;i<napp;i++){
		j=0;
		sprintf(line,"%s",appname[i]);
        sprintf(tmp,":%d",Nsize[i]);
        strcat(line,tmp);
        sprintf(tmp,":%d",NCPU[i]);
        strcat(line,tmp);
        sprintf(tmp,":%d",NCOM[i]);
        strcat(line,tmp);
        sprintf(tmp,":%d",NIO[i]);
        strcat(line,tmp);
        sprintf(tmp,":%f",IOaction[i]);
        strcat(line,tmp);
        sprintf(tmp,":%d",NumIter[i]);
        strcat(line,tmp);
        
        LOCAL_nhclasses=currclass[i];
		while(nproc[i]>0){
			if(LOCAL_ncores[LOCAL_nhclasses]>0){
				nproc[i]--;
				LOCAL_ncores[LOCAL_nhclasses]--;
				j++;
			}
			else{
				 if(j>0){
					sprintf(tmp,":%s:%d",LOCAL_rclasses[LOCAL_nhclasses],j);
					strcat(line,tmp);
				 }
                 
                  
                  if(GLOBAL_DIFFERENTNODES){
                    if(i==0) LOCAL_nhclasses++;  // New class
                    else LOCAL_nhclasses--;  // New class
                  }
                  else{
                    LOCAL_nhclasses++; 
                  }
                 
                 if(LOCAL_nhclasses<0 || LOCAL_nhclasses>=LOCAL_totclasses){
                     printf("\n Faltal error in Workload Generator:: number of existing compute-node classes exceeded \n");
                     abort();
                 }
				 j=0;
			}
		}
		sprintf(tmp,":%s:%d",LOCAL_rclasses[LOCAL_nhclasses],j);
		strcat(line,tmp);		
		printf("\n \t  %s",line);
		fprintf(file,"%s\n",line);
	}
	fclose(file);

}

// Main program		
int main (int argc, char** argv)
{
	int napp,app[1000],Nsize[1000],NIO[1000],NCPU[1000],NCOM[1000],NumIter[1000];
    double IOaction[1000];
    int n,i=1,class;
	FILE *file;
	char readline[1000];
    char appname[1000][100];
    char token[] = ":", *record = NULL;
    	
	printf("\n \n **************************************************************** \n");
	printf("      FlexMPIClarisse program workload generator 3.1\n");
	printf("\n \n **************************************************************** \n");
	
	printf("\n \n --- Initializing...\n");
    


    // Detects whether the controller runs in a exclusive compute note (the first one). This is the default option
    GLOBAL_DIFFERENTNODES=0;
	for (n = 0; n < argc; n ++) {
		if (strcmp(argv[n], "-differentnodes") == 0) {	
            i++;
            GLOBAL_DIFFERENTNODES=1;
		}
	}
	if(GLOBAL_DIFFERENTNODES==1) printf("\n ### Application processes placed in a different compute nodes\n");
    else printf("\n ### Application processes placed contiguous and in consecutive compute nodes\n");
    
    CONTROLLERPROCS=0;
    GLOBAL_EXCLCONTROLLER=1;
 	for (n = 0; n < argc; n ++) {
        if (strcmp(argv[n], "-noexcl") == 0) {	
            i++;
			GLOBAL_EXCLCONTROLLER=0;
		}
	}   

	if(GLOBAL_EXCLCONTROLLER==1) printf("\n ### Application controller executed in a different compute nodes\n");
    else printf("\n ### Application controller executed in the same compute note as the applications \n");

	if(argc-i!=2){
		printf("\n Error: wrong number of input arguments.... ");
		printf("\n Use: workloadgen [-differentnodes] [-noexcl] node_filename application_filename  "); 
        printf("\n      Argument -differentnodes executeds each application of them in different compute nodes (if possible) ");
        printf("\n      Argument -noexcl executes the controller in the same compute nodes as the applications \n");
        
		exit(1);
	}

	
     // Parses the node file
	if ((file = fopen (argv[i+1], "r")) == NULL) {
		fprintf (stderr, "\nError opening application_filename in Workload_generator %s \n",argv[2]);
		exit(1);
	}
	
	napp=0;
    printf("\n");
    
 	while (fgets (readline,1000,file) != NULL) {
        if(readline[0]!='#' && readline[0]!='\n'){

            record = strtok (readline, token);  // App name
            if(record==NULL){fprintf (stderr, "\n Error0 parsing %s file \n",argv[2]);exit(1); }    
            // Checks the if the application name is valid
            class=0;
            if(strcmp(record,"jacobi")==0) class=1;
            if(strcmp(record,"cg")==0) class=2;
            if(strcmp(record,"epigraph")==0) class=3;
            if(class==0){
                fprintf (stderr, "\n Error0 in %s file: application name not valid \n",argv[2]);
                exit(1); 
            }
            strcpy(appname[napp], record);  
            record = strtok (NULL, token);  // Num procs. 
            if(record==NULL){fprintf (stderr, "\n Error1 parsing %s file \n",argv[2]);exit(1); }         
            app[napp]=atoi(record);
            record = strtok (NULL, token);  // Size
            if(record==NULL){fprintf (stderr, "\n Error2 parsing %s file \n",argv[2]);exit(1); }
            if(class<3) Nsize[napp]=atoi(record);
            else        Nsize[napp]=0;
            record = strtok (NULL, token);  // NCPU
            if(record==NULL){fprintf (stderr, "\n Error3 parsing %s file \n",argv[2]);exit(1); }
            if(class==1) NCPU[napp]=atoi(record);
            else         NCPU[napp]=0;
            record = strtok (NULL, token);  // NCOM
            if(record==NULL){fprintf (stderr, "\n Error4 parsing %s file \n",argv[2]);exit(1); }
            if(class==1) NCOM[napp]=atoi(record);
            else         NCOM[napp]=0;
            record = strtok (NULL, token);  // NIO
            if(record==NULL){fprintf (stderr, "\n Error5 parsing %s file \n",argv[2]);exit(1); }
            if(class==1) NIO[napp]=atoi(record);
            else         NIO[napp]=0;
            record = strtok (NULL, token);  // IOaction
            if(record==NULL){fprintf (stderr, "\n Error6 parsing %s file \n",argv[2]);exit(1); }
            IOaction[napp]=atof(record);
            record = strtok (NULL, token);  // NumIter
            if(record==NULL){fprintf (stderr, "\n Error7 parsing %s file \n",argv[2]);exit(1); }
            if(class<3) NumIter[napp]=atof(record);
            else        NumIter[napp]=0;
            
            printf("    Application %d :: %s \t Np= %d \t Size= %d \t NCPU= %d \t NCOM= %d \t NIO= %d \t IOaction= %f \t NumIter= %d \n",napp,appname[napp],app[napp],Nsize[napp],NCPU[napp],NCOM[napp],NIO[napp],IOaction[napp],NumIter[napp]);
            napp++;
            if(napp>1000){
                fprintf (stderr, "\nError number of applications  is exceeded \n");
                exit(1);		 
            }
        }
	}
	fclose(file);


	Workload_generator(argv[i],appname,napp,app,Nsize,NIO,NCPU,NCOM,IOaction,NumIter);
	printf("\n\n Exiting.... \n \n");
    
	exit(1);
}
