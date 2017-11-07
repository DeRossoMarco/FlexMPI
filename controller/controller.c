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
 *	File:       controller.c																											*
 *																																		*
 ****************************************************************************************************************************************/
 
#include <time.h>
#include <sys/time.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <math.h>
#include <errno.h>
#include <netdb.h>

#include <signal.h>

#define BUFLEN 512
#define NPACK 10
#define NUMBER_OPTIONS 10
#define EMPI_COMMBUFFSIZE 2048
#define NUM_CORES 12


#define MAX_APPS  100
#define NCLASES   100
#define BUFSIZE   100
#define STAMPSIZE 10  
#define BUFFDEEP  1000000

struct  applicationset {
	char hclasses[NCLASES][128]; //host classes
	char rclasses[NCLASES][128]; //reduce clase definition
	int  nprocs_class[NCLASES];
	int  newprocs_class[NCLASES];
	int  nhclasses;
	char node[255];
	int  port1;
	int  port2;
	
	};

// Global values
static const char *HOME;

struct timeval initial;

struct applicationset GLOBAL_app[MAX_APPS];

int GLOBAL_napps, GLOBAL_RECONF, GLOBAL_PREDEFSPEEDUP,GLOBAL_SYNCIOPHASES;

int newPort1, newPort2;

pthread_mutex_t CONTROLLER_GLOBAL_server_lock;

int terminated_app;

struct arg_struct1 {
    int id;
    int port2;
};

struct arg_struct2 {
    int id;
    int *flag_speedup;
};

typedef struct EMPI_Spawn_data {
    int  dirty;  //indicates if we have fresh spawn data
    int  hostid; //host id
    int  nprocs; //max running procs
    char name [128]; //host name
} EMPI_Spawn_data;

typedef struct command_flexmpi {
    int    command_n;
    char * options[NUMBER_OPTIONS];
} command_flexmpi;

typedef struct service_arguments {
    int socket;
    struct sockaddr_in address;
} service_arguments;
 

// Function declaration
void diep(char *s);


// Kills all the active applications
void killall()
{
	int n,err,initialSocket,length;
	char initMsg[10],stringport[1000];
	sprintf(initMsg,"5:"); // This is the kill command
	
    struct addrinfo hints;
    memset(&hints,0,sizeof(hints));
    hints.ai_family=AF_UNSPEC;
    hints.ai_socktype=SOCK_DGRAM;
    hints.ai_protocol=0;
    hints.ai_flags=AI_ADDRCONFIG;
    struct addrinfo* res=0;

	
	for (n=0;n<GLOBAL_napps;n++)
	{
		// Parses the destination application
		sprintf(stringport,"%d",GLOBAL_app[n].port1);
		err=getaddrinfo(GLOBAL_app[n].hclasses[0],stringport,&hints,&res);
		if (err<0) {
			diep("failed to");
		}
		
		// Creates a socket to the destination
		initialSocket=socket(res->ai_family,res->ai_socktype,res->ai_protocol);
 		if (initialSocket==-1) {
			diep("Error creating socket.");
		}
		
		// Sends the command
		length = sendto(initialSocket,initMsg,strlen(initMsg),0,res->ai_addr,res->ai_addrlen);
		if (length < 0){
			diep("Error: sendto()");
		}
		
		// Closes the socket
		close(initialSocket);

	}
}


void intHandler(int dummy) {
    killall();
	exit(1);
}

void diep(char *s)
{
  perror(s);
  killall();
  exit(1);
}


// This function reads the workload files and creates the environment (conf files + execution scripts) and runs the applications
static void Parse_malleability (char *filename) 
{
	int  nprocs=0, n,m;
	char readline[1000], output[2000],line[2000],token[] = ":", *record = NULL;
	int port1,port2,max,maxindex,nfile,appclass=0;
	FILE *file1,*file2;
	int index[NCLASES],tmpval[NCLASES];
	char LOCAL_hclasses[NCLASES][128]; //host classes
	char LOCAL_rclasses[NCLASES][128]; //host classes
	int  LOCAL_nprocs_class[NCLASES];
	int LOCAL_nhclasses;
	
	// Initialization of the existing compute nodes
	LOCAL_nhclasses=1;
	strcpy(LOCAL_hclasses[0],"localhost");

	// Initialization of the existing compute nodes
	strcpy(LOCAL_rclasses[0],"lhost");
	
	memset(output, 0, 2000);
	
	// Initial value of the communication ports
	port1=6666; // Listener
	port2=6667; // Sender
	
	// Opens workload file
	printf("  Reading the workload ... \n");
	if ((file1 = fopen (filename, "r")) == NULL) {
		fprintf (stderr, "\nError in EMPI_Parse_malleability file1 opening\n");
		exit(1);
	}

	// Read and parse roadmap file
	nfile=0;
	while (fscanf (file1, "%s\n", readline) != EOF) {

		memset(output, 0, 2000);
		
		//Get values
		record = strtok (readline, token);
		nprocs=0;
		appclass=0;
		
		
		// First entry is the application name
		if (strcmp (record, "cpu") == 0) appclass=1;  // Jacobi cpu
		if (strcmp (record, "mem") == 0) appclass=2; // Jacobi mem
		if (strcmp (record, "vpicio") == 0) appclass=3; // cpicio
        if (strcmp (record, "com") == 0) appclass=4; // Jacobi_bench communication intensive
	    if (strcmp (record, "IO") == 0) appclass=5;  // GSL IO example
	    if (strcmp (record, "cls") == 0) appclass=6; // Clarisse IO example
	    if (strcmp (record, "jio") == 0) appclass=7; // Jacobi IO example	
		if (appclass==0) diep("\n Error format roadmap file: no valid application\n");

		// Following entries are the compute nodes and max initial processes per nodes
		while (record != NULL) {
			for (n = 0; n < LOCAL_nhclasses; n ++) {
				if (strcmp (LOCAL_hclasses[n], record) == 0) {
					record = strtok (NULL, token);
					LOCAL_nprocs_class[n] = atoi(record)*1; 
					nprocs+=LOCAL_nprocs_class[n];
				}
			}
			record = strtok (NULL, token);
		}
		
	
		// Sorts the list (ew use the permutation vector index)
		for (n = 0; n < LOCAL_nhclasses; n ++) tmpval[n]=LOCAL_nprocs_class[n];
		
		for (m = 0; m < LOCAL_nhclasses; m ++){
			max=-1;
			for (n = 0; n < LOCAL_nhclasses; n ++){
			   if(tmpval[n]>=max) {
				max=tmpval[n];
				maxindex=n;
			   }
			}
			index[m]=maxindex;
			tmpval[maxindex]=-100;
		}
		
		// Backups the list to the global structure
		GLOBAL_app[nfile].nhclasses=LOCAL_nhclasses;
		for (n = 0; n < LOCAL_nhclasses; n ++) {
			m=index[n];
			strncpy(GLOBAL_app[nfile].hclasses[n],LOCAL_hclasses[m],sizeof(LOCAL_hclasses[m]));
			strncpy(GLOBAL_app[nfile].rclasses[n],LOCAL_rclasses[m],sizeof(LOCAL_rclasses[m]));
			GLOBAL_app[nfile].nprocs_class[n]=LOCAL_nprocs_class[m];
			GLOBAL_app[nfile].port1=port1;
			GLOBAL_app[nfile].port2=port2;
			GLOBAL_app[nfile].newprocs_class[n]=0; 
		}
		
		// Increases 0 values in nproc_class (crashes MPI app)
		for (m = 0; m < LOCAL_nhclasses; m ++){
			if(LOCAL_nprocs_class[m]==0) LOCAL_nprocs_class[m]=1;
		}
		
		// Creates the output as a string
		for (m = 0; m< LOCAL_nhclasses; m ++) {
			n=index[m];
			sprintf(line,"%s:%d\n",LOCAL_hclasses[n],LOCAL_nprocs_class[n]);
			strcat(output,line);
		}
		
		// Stores the root node name
		strncpy(GLOBAL_app[nfile].node,LOCAL_hclasses[index[0]],sizeof(LOCAL_hclasses[index[0]]));
		printf(" @@@@ App [%d], root node: %s \n",nfile,GLOBAL_app[nfile].node);
		
		
		// Create rankfile
		printf("  Creating the rankfile %d ...\n",nfile+1);
		sprintf(line,"./rankfiles/rankfile%d",nfile+1);
		if ((file2 = fopen (line, "w")) == NULL) {
			fprintf (stderr, "\nError in EMPI_Parse_malleability file2 opening\n");
			exit(1);
		}
		fprintf(file2,"%s",output);
		fclose(file2);
		
		sprintf(output,"cd %s/FlexMPI/scripts\n",HOME); // For Jacobi
		sprintf(line,"export LD_LIBRARY_PATH=%s/LIBS/glpk/lib/:%s/FlexMPI/lib/:%s/LIBS/mpich/lib/:%s/LIBS/papi/lib/\n",HOME,HOME,HOME,HOME);
		strcat(output,line);
		if(appclass==6) { // Clarisse
			sprintf(line,"export CLARISSE_COUPLENESS=dynamic\n");
		    strcat(output,line);	
			sprintf(line,"export CLARISSE_PORT_PATH=%s/FlexMPI/examples/examples1\n",HOME);
		    strcat(output,line);		
		}
		if (appclass==1) sprintf(line,"./Lanza_cpu %d %d %d %d\n",nprocs,port1,port2,nfile+1);
		if (appclass==2) sprintf(line,"./Lanza_mem %d %d %d %d\n",nprocs,port1,port2,nfile+1);
		if (appclass==3) sprintf(line,"./Lanza7 %d %d %d %d\n",nprocs,port1,port2,nfile+1);
        if (appclass==4) sprintf(line,"./Lanza_com %d %d %d %d\n",nprocs,port1,port2,nfile+1);
        if (appclass==5) sprintf(line," %s/FlexMPI/examples/examples2/source/benchmarks/write/vpicio/vpicio_uni/Lanza_IO %d %d %d %d\n",HOME,nprocs,port1,port2,nfile+1);
		if (appclass==6) sprintf(line,"./Lanza_clarisse %d %d %d %d\n",nprocs,port1,port2,nfile+1);
		if (appclass==7) sprintf(line,"./Execute2.sh %d %d %d %d\n",nprocs,port1,port2,nfile+1);
		port1+=2;
		port2+=2;
		
		strcat(output,line);

		// Create execution script
		printf("  Creating the execution script %d ...\n",nfile+1);
		sprintf(line,"./execscripts/exec%d",nfile+1);
		if ((file2 = fopen (line, "w")) == NULL) {
			fprintf (stderr, "\nError in EMPI_Parse_malleability file2 opening\n");
			exit(1);
		}
		fprintf(file2,"%s",output);
		fclose(file2);
		
		sprintf(output,"chmod 755 %s/FlexMPI/controller/execscripts/exec%d",HOME,nfile+1);
		system(output);
		
		// Executes the application
		printf("  Executing the application %d ...\n",nfile+1);
		sprintf(output,"%s/FlexMPI/controller/execscripts/exec%d > ./logs/output%d 2>&1 &",HOME,nfile+1,nfile);
		if(GLOBAL_PREDEFSPEEDUP==0 || strcmp(filename,"workload")==0) system(output); // This is the system call to execute
		nfile++;
	}
	fclose(file1);
	GLOBAL_napps=nfile;
}

// Checks the return values of posix calls
void check_posix_return(int rc, char* cause)
{
    if (rc != 0)
    {
        printf("\nError: %s :[%s]", cause, strerror(rc));
    }
    else
    {
        printf("   %s successfully\n", cause);
    }
}




// Listener thread. One per application
int command_listener(void *arguments)
{
    struct sockaddr_in si_me;
    struct sockaddr_in si_other;
    int s, n, slen = sizeof(si_other),local_terminated_app;
	struct arg_struct1 *args = arguments;
	FILE *fp;
	char path[1035];
	
    char * buf     = calloc(EMPI_COMMBUFFSIZE, 1);
	char bashcmd[1024];
	
    if ((s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
    {
        diep("socket");
    }
    memset((char *) &si_me, 0, sizeof(si_me));
    si_me.sin_family = AF_INET;
    si_me.sin_port = htons(args->port2);
    si_me.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(s, (struct sockaddr *)&si_me, sizeof(si_me))==-1)
    {
        diep("bind");
    }

	// Main loop that reads the buffer values
    while(1)
    {
        memset(buf, 0, EMPI_COMMBUFFSIZE);
        int length = recvfrom(s, buf, EMPI_COMMBUFFSIZE, 0, (struct sockaddr *)&si_other, (socklen_t *)&slen);
        if (length == -1)
        {
            diep("recvfrom()");
        }
		
		if(strcmp(buf,"Aplication terminated")==0)
		{
			 printf("   %d data from %s:%d --> %s",args->id, inet_ntoa(si_other.sin_addr), ntohs(si_other.sin_port), buf);
			 pthread_mutex_lock(&CONTROLLER_GLOBAL_server_lock);
			 terminated_app++; 
			 local_terminated_app=terminated_app;
			 pthread_mutex_unlock(&CONTROLLER_GLOBAL_server_lock);	// Creates one thread per application
			 printf("\n ** Aplication terminated [%d,%d] \n",terminated_app,GLOBAL_napps);
			 
			 if(local_terminated_app>=GLOBAL_napps)
			 {
				  printf("\n *** All applications have terminated.... exiting \n\n");
				  sleep(10);		  
				  killall(); // Terminates all the applications
				  sleep(5);
				  
				  for (n=0;n<GLOBAL_napps;n++){
						/* Open the command for reading. */
						printf(" *** Application %d log:: \n",n);
						sprintf(bashcmd,"cat %s/FlexMPI/controller/output%d | grep \"Jacobi fini\" ",HOME,n+1);

						fp = popen(bashcmd, "r");
						if (fp == NULL) {
							printf("Failed to run command\n" );
							exit(1);
						}

						/* Read the output a line at a time - output it. */
						while (fgets(path, sizeof(path)-1, fp) != NULL) {
							printf("   %s \n", path);
						}

						/* close */
						pclose(fp);
						sleep(5);
					}
					exit(1);
			}
		}			
		else{
			printf("   %d data from %s:%d --> %s \n",args->id, inet_ntoa(si_other.sin_addr), ntohs(si_other.sin_port), buf);
		}		
	}
}

// Main program		
int main (int argc, char** argv)
{
	int n,m;
    char initMsg[500];
	char stringport[100];
    time_t rawtime;
    time (&rawtime);
    int length;
	struct arg_struct1 args1[MAX_APPS];
	struct arg_struct2;
	
	// Get the initial time value
	pthread_mutex_lock(&CONTROLLER_GLOBAL_server_lock);
	gettimeofday(&initial, NULL);
	pthread_mutex_unlock(&CONTROLLER_GLOBAL_server_lock);

	// Monitoring threads
	int rc;  // return code
	pthread_t thread[MAX_APPS*2+6];
	pthread_attr_t attr[MAX_APPS*2+6];
	printf("\n \n **************************************************************** \n");
	printf("      FlexMPI program controller 1.05\n");
	printf("\n \n **************************************************************** \n");

	
	printf("\n \n --- Initializing\n");

	// Home capture
	const char *name = "HOME";
	HOME=getenv(name);


	// Captures ctrl+c signal and exists killing all the apps
	signal(SIGINT, intHandler);

	GLOBAL_RECONF=1;
	for (n = 0; n < argc; n ++) {
		if (strcmp(argv[n], "-nomalleable") == 0) {
			GLOBAL_RECONF=0;
		}
	}
	
	if(GLOBAL_RECONF==1) printf("\n ### Application maleability enabled \n\n\n");
	else printf("\n ### Application maleability disabled \n\n\n");
	
	// Launches applications 
	Parse_malleability("workload");	
		
    // Configures the socket
    struct addrinfo hints;
    memset(&hints,0,sizeof(hints));
    hints.ai_family=AF_UNSPEC;
    hints.ai_socktype=SOCK_DGRAM;
    hints.ai_protocol=0;
    hints.ai_flags=AI_ADDRCONFIG;
    struct addrinfo* res=0;
	int initialSocket,err;
	
	pthread_mutex_lock(&CONTROLLER_GLOBAL_server_lock);
	terminated_app=-10*GLOBAL_napps;  // Initial value is too small to exit the program
	pthread_mutex_unlock(&CONTROLLER_GLOBAL_server_lock);	// Creates one thread per application
	
	
	printf("\n \n --- Creating the listerner threads \n");
	for(n=0;n<GLOBAL_napps;n++){ 
		args1[n].id = n;
		args1[n].port2 = GLOBAL_app[n].port2;
		rc = pthread_attr_init(&attr[n]);
		check_posix_return(rc, "Initializing attribute");
		rc = pthread_attr_setdetachstate(&attr[n], PTHREAD_CREATE_DETACHED);
		check_posix_return(rc, "Setting detached state");
		rc = pthread_create(&thread[n], &attr[n], (void*)&command_listener,(void *)(&args1[n]));
		check_posix_return(rc, "Creating listener thread ");
	}
		

	
	// Prints the application execution environment
	printf("\n \n --- Displaying the application workload\n");
	for(n=0;n<GLOBAL_napps;n++){
		printf("  Application %d, Port1 (listener): %d Port2 (Sender): %d\n",n,GLOBAL_app[n].port1,GLOBAL_app[n].port2);
		for(m=0;m<GLOBAL_app[n].nhclasses;m++){
			printf("    %s %d \n",GLOBAL_app[n].hclasses[m],GLOBAL_app[n].nprocs_class[m]);
		}
	}	
	
	
	printf("  Please type a command [appId command]: ");

	// Input commands
	printf("\n \n  --- Waiting for new input commands \n");
	while(1){
		scanf("%d %s",&n,initMsg); // First value is the application id, second value is the string with the command
		if(n==-1 || strcmp(initMsg,"exit") == 0 || strcmp(initMsg,"quit") == 0 ) 
		{
		 killall(); // Terminates all the applications
		 printf(" Exiting \n \n");
		 exit(1);
		}
		
		// Parses the destination application
		sprintf(stringport,"%d",GLOBAL_app[n].port1);
		err=getaddrinfo(GLOBAL_app[n].hclasses[0],stringport,&hints,&res);

		if (err<0) {
			diep("failed to");
		}
		
		// Creates a socket to the destination
		initialSocket=socket(res->ai_family,res->ai_socktype,res->ai_protocol);
 		if (initialSocket==-1) {
			diep("Error creating socket.");
		}
		
		// Sends the command
		length = sendto(initialSocket,initMsg,strlen(initMsg),0,res->ai_addr,res->ai_addrlen);
		if (length < 0){
			diep("Error: sendto()");
		}
		
		// Closes the socket
		close(initialSocket);

		printf("Message: %s. Size: %d bytes sent to app%d.\n",initMsg, (int)sizeof(initMsg),n+1);
	}
    return 0;
	
}
