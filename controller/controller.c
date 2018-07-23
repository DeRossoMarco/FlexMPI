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
#define NUMBER_OPTIONS 10
#define EMPI_COMMBUFFSIZE 2048

// WS1 -> Mínimum number of samples to make a prediction
// WS2 -> Maximum number of samples used in the prediction
#define MAX_APPS            50
#define WS1		            2
#define WS2		            5
#define NCLASES             500
#define BUFSIZE             512
#define STAMPSIZE           8  
#define IORATIO             5
#define NUMSAMPLES          1000

// Sampling iterations of the original code 
#define SAMPLING            100
#define reconfig_overhead   15
static const char *HOME;

struct  applicationset {
	char hclasses[NCLASES][128]; //host classes
	char rclasses[NCLASES][128]; //reduce clase definition
	int  ncores_class[NCLASES];
	int  nprocs_class[NCLASES];
	int  newprocs_class[NCLASES];
	int  nhclasses;
	char node[255];
	int  port1;
	int  port2;
    int  port3;     // GUI monitor port
    int  monitor;   // Monitor service (0 no active/1 active but not configures/2 active and configured)
    int  nsize; 
    int cpu_intensity;
    int IO_intensity;
    int com_intensity;
	};

struct timeval initial;

struct applicationset GLOBAL_app[MAX_APPS];

int GLOBAL_napps, GLOBAL_RECONF, GLOBAL_PREDEFSPEEDUP,GLOBAL_SYNCIOPHASES,GLOBAL_busyIO,GLOBAL_monitoring,GLOBAL_iter[MAX_APPS],GLOBAL_GUI,GLOBAL_GUI_PORT,GUI_ListenerPort,GLOBAL_TERMINATION;
int GLOBAL_reqIO[MAX_APPS];
int GLOBAL_EXEC,GLOBAL_ADHOC, GLOBAL_RECORDEDSPEEDUP, GLOBAL_RECORDEDEXECTIMES,GLOBAL_MAXTHROUGHPUT,GLOBAL_FAIRSCHEDULING_NP,GLOBAL_SPEEDUPSCHEDULING_NP;
int GLOBAL_CLARISSEONLY,GLOBAL_FAIRSHEDULING,GLOBAL_SPEEDUPSCHEDULING,GLOBAL_IOSCHEDULING;
char GLOBAL_FILE[1024],GLOBAL_CONTROLLER_NODE[1024],GUI_NODE[1024];
int newPort1, newPort2;

// Data structures for perf prediction
double tstamp[NUMSAMPLES][MAX_APPS],tlong[NUMSAMPLES][MAX_APPS],cput[NUMSAMPLES][MAX_APPS],delay_v[NUMSAMPLES][MAX_APPS],delay_t[NUMSAMPLES][MAX_APPS];
double GLOBAL_delayt1[MAX_APPS],GLOBAL_delayt2[MAX_APPS],GLOBAL_acqt[MAX_APPS],GLOBAL_IOSCHEDULING_THRESHOLD;  // Time samples 
int  numprocs[NUMSAMPLES][MAX_APPS],niter[NUMSAMPLES][MAX_APPS];
int cnt_app[MAX_APPS],update[MAX_APPS],cnt_delay[MAX_APPS];
double p_tstamp[STAMPSIZE][MAX_APPS],p_tlong[MAX_APPS]; // Predicted values
int napps=0; 
pthread_mutex_t CONTROLLER_GLOBAL_server_lock;
int reset[MAX_APPS];
double maxOverlap=6,uncertain=2;
 
// Performance metrics
double perf[MAX_APPS][NUMSAMPLES][8]; // perf[num_apps][num_samples][num_values]
double GLOBAL_epoch[NUMSAMPLES];
int cnt_epoch;
int cnt_perf[MAX_APPS];		   // cnt_perf[num_apps]
double exectime[MAX_APPS][BUFSIZE]; // exectime[num_apps][num_samples]
int exectorig[MAX_APPS][BUFSIZE]; // exectorig[num_apps][num_samples]
double speedup1[MAX_APPS][BUFSIZE][8],speedup2[MAX_APPS][BUFSIZE][8]; // speedup[num_apps][num_samples][num_values]
int cnt_speedup1[MAX_APPS],cnt_speedup2[MAX_APPS];			// cnt_speedup1[num_apps]
int flag_speedup[MAX_APPS];

// Global values
int Tconf=2; // Global reconfiguration time
int terminated_app;
int GLOBAL_totalcores;

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

typedef struct EMPI_Monitor_type {
    int         count;
    long long   rtime;
    long long   ptime;
    double      ctime;
    long long   flops;
    int         hostid;
    long long   flops_iteration;
    long long   it_time;
} EMPI_Monitor_type;

typedef struct service_arguments {
    int socket;
    struct sockaddr_in address;
} service_arguments;
 

 void check_node()
 {
	FILE *file;
	char command[256],nodename1[256],nodename2[256];

	printf("\n Checking the node names..... ");
	
	// Opens the nodename generated by workloadgen
	if ((file = fopen ("controller.dat", "r")) == NULL) {
		fprintf (stderr, "\nError opening controller.dat in check_node %s \n","controller.dat");
		exit(1);
	}
	fscanf (file, "%s\n", nodename1);
	fclose(file);
	
	sprintf(command,"uname -a | awk '{print $2}' > controller2.dat");
	system(command);
	
	// Opens the actual nodename 
	if ((file = fopen ("controller2.dat", "r")) == NULL) {
		fprintf (stderr, "\nError opening controller2.dat in check_node %s \n","controller2.dat");
		exit(1);
	}
	fscanf (file, "%s\n", nodename2);
	fclose(file);
	
	if(strcmp(nodename1,nodename2)!=0){
		printf(" failed!\n");
		fprintf (stderr, "\nError: Existing nodename does not math with the actual one:: %s != %s \n\n",nodename1,nodename2);
		exit(1);		
	}
	else{
		pthread_mutex_lock(&CONTROLLER_GLOBAL_server_lock);
		strcpy(GLOBAL_CONTROLLER_NODE,nodename1);
		pthread_mutex_unlock(&CONTROLLER_GLOBAL_server_lock);
		printf(" Ok\n ");
		
	}
	

 }
 
// Function declaration
void diep(char *s);


// Kills all the active applications
void killall(int id)
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
        if(id==-1 || id==n){
            // Parses the destination application
            sprintf(stringport,"%d",GLOBAL_app[n].port1);
            err=getaddrinfo(GLOBAL_app[n].node,stringport,&hints,&res);
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
            
            //sprintf(npingcmd,"nping --udp -g 5000 -p %d  -c 1 %s --data-string \"%s\">/dev/null",GLOBAL_app[n].port1,GLOBAL_app[n].node,nodes);
            //printf("\t*** %s \n",npingcmd);
            //system(npingcmd);
        }
	}
}


void intHandler(int dummy) {
    killall(-1);
	exit(1);
}

void diep(char *s)
{
  perror(s);
  killall(-1);
  exit(1);
}


int EMPI_GLOBAL_perform_load_balance;


// This function reads the workload files and creates the environment (conf files + execution scripts) and runs the applications
static void Parse_malleability (char *filename1,char  *filename2, int benchmark_mode) 
{
	int  nprocs=0, n,m;
	char readline[1000], output[2000],line[2000],token[] = ":", *record = NULL;
	int port1,port2,nfile,appclass=0;
	FILE *file1,*file2;
	int index[NCLASES],tmpval[NCLASES];
	char LOCAL_hclasses[NCLASES][128]; //host classes
	char LOCAL_rclasses[NCLASES][128]; //host classes
	int  LOCAL_nprocs_class[NCLASES];
	int  LOCAL_ncores[NCLASES];
	int LOCAL_nhclasses;
    int Nsize,NIO,NCPU,NCOM;
    
	// int maxindex,max;
	
	// Parses the node file
	printf("  Reading the nodefile ... \n");
	if ((file1 = fopen (filename2, "r")) == NULL) {
		fprintf (stderr, "\nError opening nodefile in Parse_malleability %s \n",filename2);
		exit(1);
	}

	printf("\n Node names: \t name \t \t alias \t\t #cores \n");
    

	// Read and parse node file that contains the node file names
	LOCAL_nhclasses=0;
	while (fscanf (file1, "%s\n", readline) != EOF) {
		//Get values
		record = strtok (readline, token);  // Node name
		if(record==NULL){fprintf (stderr, "\nError1 parsing Parse_malleability %s\n",filename2);exit(1); }
		strncpy(LOCAL_hclasses[LOCAL_nhclasses],record,strlen(record));
		record = strtok (NULL, token);  // Num procs. 
		if(record==NULL){fprintf (stderr, "\nError3 parsing Parse_malleability %s\n",filename2);exit(1); }
		LOCAL_ncores[LOCAL_nhclasses]=atoi(record);
		record = strtok (NULL, token);  // Node alias
		if(record==NULL){fprintf (stderr, "\nError2 parsing Parse_malleability %s\n",filename2);exit(1); }
		strncpy(LOCAL_rclasses[LOCAL_nhclasses],record,strlen(record));
		
		printf(" \t \t %s \t %s \t %d \n",LOCAL_hclasses[LOCAL_nhclasses],LOCAL_rclasses[LOCAL_nhclasses],LOCAL_ncores[LOCAL_nhclasses]);
		
		LOCAL_nhclasses++;
	}
	fclose(file1);

	
	// Computes the total number of existing cores
	GLOBAL_totalcores=0;
	for(n=0;n<NCLASES;n++){
		GLOBAL_totalcores=GLOBAL_totalcores+LOCAL_ncores[n];
	}
	
	// Initial value of the communication ports
	port1=6668+2*GLOBAL_napps; // Listener
	port2=6669+2*GLOBAL_napps; // Sender
	
    
	// Opens workload file
	printf("  Reading the workload ... \n");
	if ((file1 = fopen (filename1, "r")) == NULL) {
		fprintf (stderr, "\nError in EMPI_Parse_malleability file1 opening\n");
		exit(1);
	}


	// Read and parse workload file
	nfile=GLOBAL_napps;
	while (fscanf (file1, "%s\n", readline) != EOF) {

		memset(output, 0, 2000);
		
		// Sets the local structure
		for (n = 0; n < LOCAL_nhclasses; n ++) {
			LOCAL_nprocs_class[n] = 0; 
		}	
		
		//Get values
		record = strtok (readline, token);
		nprocs=0;
		appclass=0;
		
		
		// First entry is the application name
		if (strcmp (record, "cpu") == 0) appclass=1;  // Jacobi cpu
		if (strcmp (record, "mem") == 0) appclass=2; // Jacobi mem
		if (strcmp (record, "vpicio") == 0) appclass=3; // cpicio
        if (strcmp (record, "com") == 0) appclass=4; // Jacobi_bench communication intensive
	    if (strcmp (record, "IO") == 0) appclass=5;  // GSL IO 
	    if (strcmp (record, "cls") == 0) appclass=6; // Clarisse IO 
	    if (strcmp (record, "jio") == 0) appclass=7; // Jacobi IO 	
	    if (strcmp (record, "emjio") == 0) appclass=8; // Emulator of Jacobi IO 	
		if (appclass==0) diep("\n Error format roadmap file: no valid application\n");

        // Obtains the application size
        record = strtok (NULL, token);;
        Nsize=atoi(record);

        // Obtains the number of IO iterations
        record = strtok (NULL, token);;
        NIO=atoi(record);
        GLOBAL_app[nfile].IO_intensity=NIO;
        
        // Obtains the number of CPU iterations
        record = strtok (NULL, token);;
        NCPU=atoi(record);
        GLOBAL_app[nfile].cpu_intensity=NCPU;
        
        // Obtains the number of COMMUNICATION iterations
        record = strtok (NULL, token);
        NCOM=atoi(record);
        GLOBAL_app[nfile].com_intensity=NCOM;
        
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
		
	
		// Sorts the list (use the permutation vector index)
		for (n = 0; n < LOCAL_nhclasses; n ++) tmpval[n]=LOCAL_nprocs_class[n];

        // Orders the entries by process number -> produces wrong process-node mapping in bebop
        /*
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
		*/
        
        
        m=0;
        while(m < LOCAL_nhclasses){
            
            // Takes the nodes with >0 procs. Nodes are in original order (requisite for bebop)
			for (n = 0; n < LOCAL_nhclasses; n ++){
			    if(tmpval[n]>0) {
                    index[m]=n;
                    m++;
			    }
			}
  
            // Takes the rest of them
 			for (n = 0; n < LOCAL_nhclasses; n ++){
			    if(tmpval[n]==0) {
                    index[m]=n;
                    m++;
			    }
			}			        
		}
		
		// Backups the list to the global structure
		GLOBAL_app[nfile].nhclasses=LOCAL_nhclasses;
		GLOBAL_app[nfile].nsize=Nsize;
		
		for (n = 0; n < LOCAL_nhclasses; n ++) {
			//m=index[n];
			m=n;
			strncpy(GLOBAL_app[nfile].hclasses[n],LOCAL_hclasses[m],sizeof(LOCAL_hclasses[m]));
			strncpy(GLOBAL_app[nfile].rclasses[n],LOCAL_rclasses[m],sizeof(LOCAL_rclasses[m]));
			GLOBAL_app[nfile].ncores_class[n]=LOCAL_ncores[m];
			GLOBAL_app[nfile].nprocs_class[n]=LOCAL_nprocs_class[m];
			GLOBAL_app[nfile].port1=port1;
			GLOBAL_app[nfile].port2=port2;
            GLOBAL_app[nfile].port3=-1;
            GLOBAL_app[nfile].monitor=0;
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
		printf("      App [%d], root node: %s \n",nfile,GLOBAL_app[nfile].node);
		
		
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
		sprintf(line,"export LD_LIBRARY_PATH=%s/LIBS/glpk/lib/:%s/FlexMPI/lib/:%s/LIBS/mpichbis/lib/:%s/LIBS/papi/lib/:$LD_LIBRARY_PATH\n",HOME,HOME,HOME,HOME);
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
		if (appclass==7) sprintf(line,"./Lanza_Jacobi_IO.sh %d %d %d %d %d %d %d %d\n",nprocs,port1,port2,nfile+1,Nsize,NIO,NCPU,NCOM); //Blues
		if (appclass==8) sprintf(line,"./Lanza_JacobiEmulator_IO.sh %d %d %d %d\n",nprocs,port1,port2,nfile+1);
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

		sprintf(output,"chmod 755 ./execscripts/exec%d",nfile+1);
		system(output);
		
		// Executes the application
		printf("  Executing the application %d ...\n",nfile+1);
		sprintf(output,"%s/FlexMPI/controller/execscripts/exec%d > ./logs/output%d 2>&1 &",HOME,nfile+1,nfile+1);
		if((GLOBAL_EXEC==1 &&  benchmark_mode!=1 ) || GLOBAL_ADHOC>0){
			system(output); // This is the system call to execute
			//sleep(10);
		}
		nfile++;
	}
	fclose(file1);
	GLOBAL_napps=nfile;
	
	if(GLOBAL_EXEC==0){
		printf("\n\n Execution scripts created.... terminating application.\n\n");
		exit(1);
	}
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
    
    int s, slen = sizeof(si_other),id,local_terminated_app;
	struct arg_struct1 *args = arguments;
	double rtime,ptime,ctime,mflops,count1,count2,iotime,size;
	char *token;
	
    char * buf     = calloc(EMPI_COMMBUFFSIZE, 1);
       
	
    // Configures the socket reception
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
	id=args->id;
    while(1)
    {
        memset(buf, 0, EMPI_COMMBUFFSIZE);
        int length = recvfrom(s, buf, EMPI_COMMBUFFSIZE, 0, (struct sockaddr *)&si_other, (socklen_t *)&slen);
        printf("\n Incoming message: %s",buf);
		fflush(stdout);
		if (length == -1)
        {
            diep("recvfrom()");
        }
		
		if(strcmp(buf,"Aplication terminated")==0)
		{
		 printf("   	%d termination data from %s:%d --> %s \n",args->id, inet_ntoa(si_other.sin_addr), ntohs(si_other.sin_port), buf);
		 pthread_mutex_lock(&CONTROLLER_GLOBAL_server_lock);
		 terminated_app++; 
		 local_terminated_app=terminated_app;
	     pthread_mutex_unlock(&CONTROLLER_GLOBAL_server_lock);	// Creates one thread per application
		 printf("\n ** Aplication terminated [%d,%d] \n",terminated_app,GLOBAL_napps);
		 
		 // ****** Kills all apps when one terminates
		  //killall(-1); // Terminates all the applications
		  //sleep(5);
		  //exit(1); 
		 // ******
		 if(local_terminated_app>=GLOBAL_napps)
		 {
			  printf("\n *** All applications have terminated.... exiting \n\n");
			  sleep(10);		  
			  killall(-1); // Terminates all the applications
			  sleep(5);
			  
			  exit(1);
		 }
		} 
		else{
            
			//printf("   			App idd %d sends data from IP %s:%d --> %s",args->id, inet_ntoa(si_other.sin_addr), ntohs(si_other.sin_port), buf);
			if(strlen(buf)>12 && buf[0]=='F' && buf[1]=='L' && buf[2]=='X'){
				token = strtok(buf, " ");
				if(token!=NULL)  token = strtok(NULL, " ");
				
					  token = strtok(NULL, " ");
					  token = strtok(NULL, " ");
					  rtime = atof(token);
					  token = strtok(NULL, " ");
					  token = strtok(NULL, " ");
					  ptime = atof(token);
					  token = strtok(NULL, " ");
					  token = strtok(NULL, " ");
					  ctime = atof(token);
					  token = strtok(NULL, " ");
					  token = strtok(NULL, " ");
					  mflops= atof(token);
					  token = strtok(NULL, " ");
					  token = strtok(NULL, " ");
					  count1= atof(token);
					  token = strtok(NULL, " ");
					  token = strtok(NULL, " ");
					  count2= atof(token);
					  token = strtok(NULL, " ");
					  token = strtok(NULL, " ");
					  iotime= atof(token);
					  token = strtok(NULL, " ");
					  token = strtok(NULL, " ");
					  size= atof(token);
					  pthread_mutex_lock(&CONTROLLER_GLOBAL_server_lock);
					  if(rtime!=0){  // Only counts if nnz values are obtained (real measurenmentnment)
						  perf[id][cnt_perf[id]][0]=rtime;
						  perf[id][cnt_perf[id]][1]=ptime;
						  perf[id][cnt_perf[id]][2]=ctime;
						  perf[id][cnt_perf[id]][3]=iotime;
						  perf[id][cnt_perf[id]][4]=mflops;
						  perf[id][cnt_perf[id]][5]=count1;
						  perf[id][cnt_perf[id]][6]=count2;
						  perf[id][cnt_perf[id]][7]=size;
						  cnt_perf[id]++;
						  if(cnt_perf[id]>NUMSAMPLES){ // Maximum number of event
                               killall(-1); // Terminates all the applications
                               printf(" Maximum entry of cnt_perf buffer reached... exitting  \n \n");
                               exit(1);
						  }
					  }
					  pthread_mutex_unlock(&CONTROLLER_GLOBAL_server_lock);

					printf(" 			App %d, Metrics colleted:  %.4f %.4f %.4f %.4f %.4f %.4f %.4f %.4f\n",args->id,rtime,ptime,ctime,mflops,count1,count2,iotime,size);
                    
 			}	
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
    
	// Get the initial time value
	pthread_mutex_lock(&CONTROLLER_GLOBAL_server_lock);
	gettimeofday(&initial, NULL);
	pthread_mutex_unlock(&CONTROLLER_GLOBAL_server_lock);

	// Monitoring threads
	int rc;  // return code
	pthread_t thread[MAX_APPS*2+8];
	pthread_attr_t attr[MAX_APPS*2+8];
	printf("\n \n **************************************************************** \n");
	printf("      FlexMPI program controller 2.05\n");
	printf("\n \n **************************************************************** \n");
	
	printf("\n \n --- Initializing\n");

	// Home capture
	const char *name = "HOME";
	HOME=getenv(name);
	
	// Checks that the current node is the same as the one stored in controller.dat
	check_node();
	
	// Captures ctrl+c signal and exists killing all the apps
	signal(SIGINT, intHandler);



	pthread_mutex_lock(&CONTROLLER_GLOBAL_server_lock);
	
	for(n=0;n<MAX_APPS;n++){
		GLOBAL_reqIO[n]=0;
		GLOBAL_acqt[n]=0;
		GLOBAL_delayt1[n]=0;
		GLOBAL_delayt2[n]=0;
	}
	
    GLOBAL_TERMINATION=0;
	GLOBAL_RECONF=1;
	GLOBAL_busyIO=0;
    GLOBAL_napps=0;
    
    
	for (n = 0; n < argc; n ++) {
		if (strcmp(argv[n], "-nomalleable") == 0) {
			GLOBAL_RECONF=0;
		}
	}
	pthread_mutex_unlock(&CONTROLLER_GLOBAL_server_lock);
	
	if(GLOBAL_RECONF==1) printf("\n ### Application maleability enabled \n\n\n");
	else printf("\n ### Application maleability disabled \n\n\n");

	// if flag -noexecute is present: only generates exec scripts (it does not execute the application)
	GLOBAL_EXEC=1;
	for (n = 0; n < argc; n ++) {
		if (strcmp(argv[n], "-noexecute") == 0) {
			GLOBAL_EXEC=0;
		}
	}
	if(GLOBAL_EXEC==0) printf("\n ### Generating the execution scripts but not runnning the application \n\n\n");
	
	// if flag -adhoc is present: only spawns the given number of processes and then exits. 
	GLOBAL_ADHOC=0;
	for (n = 0; n < argc; n ++) {
		if (strcmp(argv[n], "-adhoc") == 0 && GLOBAL_EXEC==1 && GLOBAL_RECONF==1) {
			GLOBAL_ADHOC=atoi(argv[n+1]);
		}
	}
	if(GLOBAL_ADHOC>0) printf("\n ### Performing an ad-hoc dynamic configuration and exiting.... \n\n\n");
	
	GLOBAL_monitoring=0;
	for (n = 0; n < argc; n ++) {
		if (strcmp(argv[n], "-monitoring") == 0 && GLOBAL_EXEC==1) {
			GLOBAL_monitoring=1;
		}
	}
	if(GLOBAL_monitoring==1) printf("\n ### Activating global monitoring.... \n\n\n");
	
	
	GLOBAL_PREDEFSPEEDUP=0;
	for (n = 0; n < argc; n ++) {
		if (strcmp(argv[n], "-predefinedspeedup") == 0) {			
			GLOBAL_PREDEFSPEEDUP=1;
		}
	}
	if(GLOBAL_PREDEFSPEEDUP==1) printf("\n ### Application becnhmarking not activated \n\n\n");

	
	GLOBAL_RECORDEDSPEEDUP=0;
	for (n = 0; n < argc; n ++) {
		if (strcmp(argv[n], "-recordedspeedup") == 0) {			
			GLOBAL_RECORDEDSPEEDUP=1;
			strcpy(GLOBAL_FILE,argv[n+1]);
		}
	}
	if(GLOBAL_RECORDEDSPEEDUP==1) printf("\n ### Speedup loaded from file %s \n\n\n",GLOBAL_FILE);
	
	GLOBAL_RECORDEDEXECTIMES=0;
	for (n = 0; n < argc; n ++) {
		if (strcmp(argv[n], "-recordedexectime") == 0) {			
			GLOBAL_RECORDEDEXECTIMES=1;
			strcpy(GLOBAL_FILE,argv[n+1]);
		}
	}
	if(GLOBAL_RECORDEDEXECTIMES==1) printf("\n ### Execution times loaded from file %s \n\n\n",GLOBAL_FILE);

	GLOBAL_MAXTHROUGHPUT=0;
	for (n = 0; n < argc; n ++) {
		if (strcmp(argv[n], "-maxthroughput") == 0) {			
			GLOBAL_MAXTHROUGHPUT=1;
		}
	}
	if(GLOBAL_MAXTHROUGHPUT==1) printf("\n ### Maximum throughput policy activated \n\n\n");

    GLOBAL_CLARISSEONLY=0;
	for (n = 0; n < argc; n ++) {
		if (strcmp(argv[n], "-clarisseonly") == 0) {			
			GLOBAL_CLARISSEONLY=1;
		}
	}
	if(GLOBAL_CLARISSEONLY==1) printf("\n ### Running only with Clarisse. Malleability disabled \n\n\n");

	
	if(GLOBAL_RECORDEDSPEEDUP+GLOBAL_RECORDEDEXECTIMES>1){
		diep("Error: flags -recordedspeedup and -recordedexectime cannot be used at the same time \n");
	} 
    	
	GLOBAL_SYNCIOPHASES=0;
	for (n = 0; n < argc; n ++) {
		if (strcmp(argv[n], "-synciophases") == 0) {			
			GLOBAL_SYNCIOPHASES=1;
		}
	}
	if(GLOBAL_SYNCIOPHASES==1) printf("\n ### I/O period phases adjusted \n\n\n");

    GLOBAL_FAIRSHEDULING=0;
	for (n = 0; n < argc; n ++) {
		if (strcmp(argv[n], "-fair_scheduling") == 0) {			
			GLOBAL_FAIRSHEDULING=1;
            GLOBAL_FAIRSCHEDULING_NP=atoi(argv[n+1]);
		}
	}
    
	if(GLOBAL_FAIRSHEDULING==1 && GLOBAL_FAIRSCHEDULING_NP==-1) printf("\n ### Fair sheduler activated using all the available processors \n\n\n");
	if(GLOBAL_FAIRSHEDULING==1 && GLOBAL_FAIRSCHEDULING_NP>=0) printf("\n ### Fair sheduler activated using %d processors \n\n\n",GLOBAL_FAIRSCHEDULING_NP);

    GLOBAL_SPEEDUPSCHEDULING=0;
 	for (n = 0; n < argc; n ++) {
		if (strcmp(argv[n], "-speedup_scheduling") == 0) {			
			GLOBAL_SPEEDUPSCHEDULING=1;
            GLOBAL_SPEEDUPSCHEDULING_NP=atoi(argv[n+1]);
		}
	}
	if(GLOBAL_SPEEDUPSCHEDULING==1 && GLOBAL_SPEEDUPSCHEDULING_NP==-1) printf("\n ### Speedup-based sheduler activated using all the available processors \n\n\n");
	if(GLOBAL_SPEEDUPSCHEDULING==1 && GLOBAL_SPEEDUPSCHEDULING_NP>=0) printf("\n ### Speedup-based sheduler activated using %d processors \n\n\n",GLOBAL_SPEEDUPSCHEDULING_NP);
    
    GLOBAL_IOSCHEDULING=0;
    GLOBAL_IOSCHEDULING_THRESHOLD=-1;
 	for (n = 0; n < argc; n ++) {
		if (strcmp(argv[n], "-io_scheduling") == 0) {			
			GLOBAL_IOSCHEDULING=1;
            GLOBAL_IOSCHEDULING_THRESHOLD=atof(argv[n+1]);
            if(GLOBAL_IOSCHEDULING_THRESHOLD<0){
                diep("Error with -io_scheduling val argument. Value should be within the interval [0,1]");
            }
		}
	}
	if(GLOBAL_IOSCHEDULING==1) printf("\n ### IO-based sheduler activated \n\n\n");
   
	if(GLOBAL_IOSCHEDULING+GLOBAL_SPEEDUPSCHEDULING+GLOBAL_FAIRSHEDULING+GLOBAL_CLARISSEONLY+GLOBAL_MAXTHROUGHPUT+GLOBAL_SYNCIOPHASES>1){
		diep("Error: flags -clarisseonly -maxthroughput and -synciophases cannot be used at the same time \n");
	} 

	GLOBAL_GUI=0;
	for (n = 0; n < argc; n ++) {
		if (strcmp(argv[n], "-GUI") == 0) {			
			GLOBAL_GUI=1;
			strcpy(GUI_NODE,argv[n+1]);
            GLOBAL_GUI_PORT=atoi(argv[n+2]);
            GUI_ListenerPort=atoi(argv[n+3]);;
		}
	}
	if(GLOBAL_GUI==1) printf("\n ### Graphic user interface node located in %s with port1: %d \n\n\n",GUI_NODE,GLOBAL_GUI_PORT);
    
	// Variable initialization
	pthread_mutex_lock(&CONTROLLER_GLOBAL_server_lock);
    
    GLOBAL_epoch[0]=0;
    cnt_epoch=2; // Points to the next-to-used entry
    
	for(n=0;n<MAX_APPS;n++)
	{
 	 cnt_perf[n]=0;
	 cnt_speedup1[n]=0;
	 cnt_speedup2[n]=0;
	 flag_speedup[n]=0;
    }	
	pthread_mutex_unlock(&CONTROLLER_GLOBAL_server_lock);
	
	// Launches applications in benchmark mode
	
	Parse_malleability("workload.dat","../run/nodefile2.dat",GLOBAL_RECORDEDSPEEDUP+GLOBAL_RECORDEDEXECTIMES);
	
	// Prints the application execution environment
	printf("\n \n --- Displaying the application workload in benchmark mode\n");
	for(n=0;n<GLOBAL_napps;n++){
		printf("  Application (benchmark) %d, Port1 (listener): %d Port2 (Sender): %d\n",n,GLOBAL_app[n].port1,GLOBAL_app[n].port2);
		for(m=0;m<GLOBAL_app[n].nhclasses;m++){
			printf(" \t \t %s \t ncores: \t %d \t nprocs \t %d\n",GLOBAL_app[n].hclasses[m],GLOBAL_app[n].ncores_class[m],GLOBAL_app[n].nprocs_class[m]);
		}
	}

	// Initialize shared variables. Threads not created yet.
	pthread_mutex_lock(&CONTROLLER_GLOBAL_server_lock);
	for(n=0;n<MAX_APPS;n++){
		cnt_app[n]=0;
		cnt_delay[n]=0;
		update[n]=0;
		p_tlong[n]=0;
		for(m=0;m<STAMPSIZE;m++){
			p_tstamp[m][n]=0;

		}
	}
	pthread_mutex_unlock(&CONTROLLER_GLOBAL_server_lock);
		
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
	
	// Creates listener threads
	printf("\n \n --- Creating the listerner threads \n");
	for(n=0;n<GLOBAL_napps;n++){ 
		args1[n].id = n;
		args1[n].port2 = GLOBAL_app[n].port2;
		rc = pthread_attr_init(&attr[n]);
		check_posix_return(rc, "Initializing attribute");
		rc = pthread_create(&thread[n], &attr[n], (void*)&command_listener,(void *)(&args1[n]));
		check_posix_return(rc, "Creating listener thread ");
        sleep(10); // ToDo: REMOCE
	}

	printf("\n Entering execution phase ..... \n\n");
		
	

	
	// Sets the initial time value (for the new application)
	pthread_mutex_lock(&CONTROLLER_GLOBAL_server_lock);
	gettimeofday(&initial, NULL);
	pthread_mutex_unlock(&CONTROLLER_GLOBAL_server_lock);
	
	// Launches applications 
	printf("\n Executing the application.... \n");
    GLOBAL_napps=0;
	Parse_malleability("workload.dat","../run/nodefile2.dat",0);
	
	
	// Prints the application execution environment
	printf("\n \n --- Displaying the application workload \n");
	for(n=0;n<GLOBAL_napps;n++){
		printf("  Application (benchmark) %d, Port1 (listener): %d Port2 (Sender): %d\n",n,GLOBAL_app[n].port1,GLOBAL_app[n].port2);
		for(m=0;m<GLOBAL_app[n].nhclasses;m++){
			printf(" \t \t %s \t ncores: \t %d \t nprocs \t %d\n",GLOBAL_app[n].hclasses[m],GLOBAL_app[n].ncores_class[m],GLOBAL_app[n].nprocs_class[m]);
		}
	}	
	pthread_mutex_lock(&CONTROLLER_GLOBAL_server_lock);	
	for(n=0;n<MAX_APPS;n++)
	{
     cnt_perf[n]=0;
	 cnt_app[n]=0;
	 update[n]=0;
	}
	pthread_mutex_unlock(&CONTROLLER_GLOBAL_server_lock);
	
	
	printf("  Please type a command [appId command]: ");

	// Input commands
	printf("\n \n  --- Waiting for new input commands \n");
	
	fflush(NULL);
	
	while(1)
	{
		scanf("%d %s",&n,initMsg); // First value is the application id, second value is the string with the command
		if(n==-1 || strcmp(initMsg,"exit") == 0 || strcmp(initMsg,"quit") == 0 ) 
		{
             killall(-1); // Terminates all the applications
             printf(" Exiting \n \n");
             exit(1);
		}
		else{
               
            // Parses the destination application
            sprintf(stringport,"%d",GLOBAL_app[n].port1);
            err=getaddrinfo(GLOBAL_app[n].node,stringport,&hints,&res);

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

            printf("Message: %s  Size: %d bytes sent to app%d at %s with port %d\n",initMsg, (int)sizeof(initMsg),n+1,GLOBAL_app[n].node,GLOBAL_app[n].port1);
        }
	}
	
	printf("\n Ending..... \n\n");
	killall(-1);
    return 0;
	
}