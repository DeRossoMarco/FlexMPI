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
 *                                                                                                                                      *
 *  FLEX-MPI                                                                                                                            *
 *                                                                                                                                      *
 *  File:       server.c                                                                                                                *
 *                                                                                                                                      *
 ****************************************************************************************************************************************/

#include <empi.h>

void diep(char *s)
{
  perror(s);
  exit(1);
}

void check_posix_return(int rc, char* cause)
{
    if (rc != 0)
    {
        printf("\nError: %s :[%s]", cause, strerror(rc));
    }
    else
    {
        printf("\n[DEBUG] %s successfully\n", cause);
    }
}

command_flexmpi parse_command(char * raw_command)
{
    char * com;
    command_flexmpi command;
    const char s = ':';

    /* get the first token */
    com = strtok(raw_command, &s);
    command.command_n = atoi(com);
    /* walk through other tokens */
    int i = 0;
    do
    {
        if(i>=NUMBER_OPTIONS){
			diep("Error parsing the message. Increase NUMBER_OPTIONS value.");
		}
		
		command.options[i] = strtok(NULL, &s);
        i++;
		

    } while (command.options[i - 1] != NULL);

    return command;
}


int service_poster(void* args)
{
	char line[1024];
	char * buf     = calloc(EMPI_COMMBUFFSIZE, 1);
	int index,length,size;
		
	struct addrinfo hints;
    memset(&hints,0,sizeof(hints));
    hints.ai_family=AF_UNSPEC;
    hints.ai_socktype=SOCK_DGRAM;
    hints.ai_protocol=0;
    hints.ai_flags=AI_ADDRCONFIG;
    struct addrinfo* res=0;

	sprintf(line,"%d",EMPI_GLOBAL_sendport);
    int err=getaddrinfo("tucan",line,&hints,&res);
	int active=1,termination=0;
	
    if (err<0) {
    diep("failed to");
    }

	int initialSocket=socket(res->ai_family,res->ai_socktype,res->ai_protocol);
    if (initialSocket==-1) {
    diep("Error creating socket.");
    }
		
    while (active)
    {
		pthread_mutex_lock(&EMPI_GLOBAL_server_lock);
		active=EMPI_GLOBAL_posteractive; // Check for service_poster termination
		termination=EMPI_GLOBAL_monitoring_data.termination; // Check por application termination
		pthread_mutex_unlock(&EMPI_GLOBAL_server_lock);

        memset(buf, 0, EMPI_COMMBUFFSIZE);
		memset(line, 0, 1024);
        EMPI_host_type* hostlist = EMPI_GLOBAL_hostlist;
		
		if(termination==0){
			//for(index = 0; index < EMPI_GLOBAL_nhosts; index++)
			for(index = 0; index < 1; index++)
			{
				char line_aux[128];
				pthread_mutex_lock(&EMPI_GLOBAL_server_lock);
				MPI_Comm_size(EMPI_COMM_WORLD,&size);
				if(EMPI_GLOBAL_monitoring_data.rtime!=0) sprintf(line, " [%s] rtime %lld\t ptime %lld\t ctime %f\t Mflops %lld\t %s %lld\t %s %lld Ratio=%f\t iotime %f size %d \n", hostlist->hostname, EMPI_GLOBAL_monitoring_data.rtime, EMPI_GLOBAL_monitoring_data.ptime, EMPI_GLOBAL_monitoring_data.ctime,(long long int)(EMPI_GLOBAL_monitoring_data.flops/EMPI_GLOBAL_monitoring_data.rtime),EMPI_GLOBAL_monitoring_data.nhwpc_1,EMPI_GLOBAL_monitoring_data.hwpc_1,EMPI_GLOBAL_monitoring_data.nhwpc_2,EMPI_GLOBAL_monitoring_data.hwpc_2,((double)EMPI_GLOBAL_monitoring_data.hwpc_2)/((double)EMPI_GLOBAL_monitoring_data.hwpc_1),EMPI_GLOBAL_monitoring_data.iotime,size);	
				
				else sprintf(line, " [%s] rtime %lld\t ptime %lld\t ctime %f\t Mflops %d\t %s %lld\t %s %lld Ratio=%f\t iotime %f size %d\n", hostlist->hostname, EMPI_GLOBAL_monitoring_data.rtime, EMPI_GLOBAL_monitoring_data.ptime, EMPI_GLOBAL_monitoring_data.ctime,1,EMPI_GLOBAL_monitoring_data.nhwpc_1,EMPI_GLOBAL_monitoring_data.hwpc_1,EMPI_GLOBAL_monitoring_data.nhwpc_2,EMPI_GLOBAL_monitoring_data.hwpc_2,((double)EMPI_GLOBAL_monitoring_data.hwpc_2)/((double)EMPI_GLOBAL_monitoring_data.hwpc_1),EMPI_GLOBAL_monitoring_data.iotime,size);	
			
				pthread_mutex_unlock(&EMPI_GLOBAL_server_lock);
				hostlist=hostlist->next;
				strcat(line, line_aux);
			}
		}
		
		if(termination==1){
			sprintf(line, "  Application terminated");
			
			// Shutdown this thread
		    pthread_mutex_lock(&EMPI_GLOBAL_server_lock);
			EMPI_GLOBAL_posteractive=0;
			active=0;
			pthread_mutex_unlock(&EMPI_GLOBAL_server_lock);
		}
		
        strncpy(buf, line, strlen(line));

        //int length = sendto(s, buf, strlen(buf), 0, (struct sockaddr *)&arguments.address, (socklen_t)sizeof(arguments.address));
		length = sendto(initialSocket,buf,strlen(buf),0,res->ai_addr,res->ai_addrlen);
		if (length == -1)
        {
            diep("sendto()");
        }
        printf("Sent as response:\n%s \n", buf);

        if (termination==0) sleep(5);
    }
	pthread_exit(&EMPI_GLOBAL_posteractive);
}

int command_listener(void)
{
    struct sockaddr_in si_me;
    struct sockaddr_in si_other;
    int i, n, s, slen = sizeof(si_other);
	size_t len0,len1;

    //int buffer_length = sizeof(int);
    char * buf     = calloc(EMPI_COMMBUFFSIZE, 1);
    char * buf_res = calloc(EMPI_COMMBUFFSIZE, 1);

    if ((s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
    {
        diep("socket");
    }

    memset((char *) &si_me, 0, sizeof(si_me));
    si_me.sin_family = AF_INET;
    si_me.sin_port = htons(EMPI_GLOBAL_recvport);
    si_me.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(s, (struct sockaddr *)&si_me, sizeof(si_me))==-1)
    {
        diep("bind");
    }

    while(1)
    {
        memset(buf, 0, EMPI_COMMBUFFSIZE);
        memset(buf_res, 0, EMPI_COMMBUFFSIZE);
        int length = recvfrom(s, buf, EMPI_COMMBUFFSIZE, 0, (struct sockaddr *)&si_other, (socklen_t *)&slen);
        if (length == -1)
        {
            diep("recvfrom()");
        }
        printf("Received packet from %s:%d   Data: %s\n\n", inet_ntoa(si_other.sin_addr), ntohs(si_other.sin_port), buf);

        //truncate message
        char bufer_cropped [length];
        strncpy(bufer_cropped, buf, length);
		strcat(bufer_cropped, "\0"); // David: add termination string
        //Parse string
        command_flexmpi command = parse_command(bufer_cropped);
        printf(" Command number is %d\n", command.command_n);
        switch(command.command_n)
        {
            case 1: //command: 1:policy:t_obj:threshold
            {

                if (strcmp(command.options[0], "EFFICIENCY") == 0)
                {
                    EMPI_Set_policy(EMPI_EFFICIENCY);
                    //Set objective
                    EMPI_GLOBAL_obj_texec = (atoi(command.options[1]));

                    //Set threshold
                    EMPI_GLOBAL_obj_texec_threshold = (atof(command.options[2]));

                    //enable load balancing
                    EMPI_Enable_lbalance ();

                    printf("policy: %s - obj = %f - threshold = %f\n", command.options[0], EMPI_GLOBAL_obj_texec, EMPI_GLOBAL_obj_texec_threshold);
                    // printf("policy: %s - obj = %d - threshold = %f\n", command.options[0], obj_texec, obj_texec_threshold);
                    sprintf(buf_res, "policy: %s - obj = %f - threshold = %f\n", command.options[0], EMPI_GLOBAL_obj_texec, EMPI_GLOBAL_obj_texec_threshold);
                    // sprintf(buf_res, "policy: %s - obj = %d - threshold = %f\n", command.options[0], obj_texec, obj_texec_threshold);

                }
                else if (strcmp(command.options[0], "EFFICIENCY_IRR") == 0)
                {
                    EMPI_Set_policy(EMPI_EFFICIENCY_IRR);
                    //Set objective
                    EMPI_GLOBAL_percentage = (atoi(command.options[1]));

                    //Set threshold
                    EMPI_GLOBAL_obj_texec_threshold = (atof(command.options[2]));

                    //enable load balancing
                    EMPI_Enable_lbalance ();

                    printf("policy: %s - obj = %f - threshold = %f\n", command.options[0], EMPI_GLOBAL_percentage, EMPI_GLOBAL_obj_texec_threshold);
                    // printf("policy: %s - obj = %d - threshold = %f\n", command.options[0], obj_texec, obj_texec_threshold);
                    sprintf(buf_res, "policy: %s - obj = %f - threshold = %f\n", command.options[0], EMPI_GLOBAL_percentage, EMPI_GLOBAL_obj_texec_threshold);
                    // sprintf(buf_res, "policy: %s - obj = %d - threshold = %f\n", command.options[0], obj_texec, obj_texec_threshold);

                }
                else if (strcmp(command.options[0], "COST") == 0)
                {
                    EMPI_Set_policy(EMPI_COST);
                    //Set objective
                    EMPI_GLOBAL_obj_texec = (atoi(command.options[1]));

                    //Set threshold
                    EMPI_GLOBAL_obj_texec_threshold = (atof(command.options[2]));

                    //enable load balancing
                    EMPI_Enable_lbalance ();

                    printf("policy: %s - obj = %f - threshold = %f\n", command.options[0], EMPI_GLOBAL_obj_texec, EMPI_GLOBAL_obj_texec_threshold);
                    // printf("policy: %s - obj = %d - threshold = %f\n", command.options[0], obj_texec, obj_texec_threshold);
                    sprintf(buf_res, "policy: %s - obj = %f - threshold = %f\n", command.options[0], EMPI_GLOBAL_obj_texec, EMPI_GLOBAL_obj_texec_threshold);
                    // sprintf(buf_res, "policy: %s - obj = %d - threshold = %f\n", command.options[0], obj_texec, obj_texec_threshold);

                }
                else if (strcmp(command.options[0], "COST_IRR") == 0)
                {
                    EMPI_Set_policy(EMPI_COST_IRR);

                    //Set objective
                    EMPI_GLOBAL_percentage = (atoi(command.options[1]));

                    //Set threshold
                    EMPI_GLOBAL_obj_texec_threshold = (atof(command.options[2]));

                    //enable load balancing
                    EMPI_Enable_lbalance ();

                    printf("policy: %s - obj = %f - threshold = %f\n", command.options[0], EMPI_GLOBAL_percentage, EMPI_GLOBAL_obj_texec_threshold);
                    // printf("policy: %s - obj = %d - threshold = %f\n", command.options[0], obj_texec, obj_texec_threshold);
                    sprintf(buf_res, "policy: %s - obj = %f - threshold = %f\n", command.options[0], EMPI_GLOBAL_percentage, EMPI_GLOBAL_obj_texec_threshold);
                    // sprintf(buf_res, "policy: %s - obj = %d - threshold = %f\n", command.options[0], obj_texec, obj_texec_threshold);

                }

                else if (strcmp(command.options[0], "LBALANCE") == 0)
                {
                    EMPI_Set_policy(EMPI_LBALANCE);
                    //enable load balancing
                    EMPI_Enable_lbalance ();
                    printf("policy: %s\n", command.options[0]);
                    sprintf(buf_res, "policy: %s\n", command.options[0]);

                }
                else if (strcmp(command.options[0], "MONITORDBG") == 0)
                {
                    EMPI_Set_policy(EMPI_MONITORDBG);
                    //enable load balancing
                    EMPI_Enable_lbalance ();
                    printf("policy: %s\n", command.options[0]);
                    sprintf(buf_res, "policy: %s\n", command.options[0]);

                }
                else if (strcmp(command.options[0], "MALLEABILITY") == 0)
                {
                    EMPI_Set_policy(EMPI_MALLEABILITY);
                    //enable load balancing
                    EMPI_Enable_lbalance ();
                    printf("policy: %s\n", command.options[0]);
                    sprintf(buf_res, "policy: %s\n", command.options[0]);

                }
                else if (strcmp(command.options[0], "MALLEABILITY_COND") == 0)
                {
                    EMPI_Set_policy(EMPI_MALLEABILITY_COND);
                    //enable load balancing
                    EMPI_Enable_lbalance ();
                    printf("policy: %s\n", command.options[0]);
                    sprintf(buf_res, "policy: %s\n", command.options[0]);

                }
                else
                {
                    printf("ERROR: Malleability policy %s NOT FOUND!", command.options[0]);
                    sprintf(buf_res, "ERROR: Malleability policy %s NOT FOUND!", command.options[0]);
                }
                break;

            }
            case 2:
                //command: 2:
                //Establish a flag to perform a load balancing at the end of the given sampling interval
                pthread_mutex_lock(&EMPI_GLOBAL_server_lock);
                EMPI_GLOBAL_perform_load_balance = 1;
                pthread_mutex_unlock(&EMPI_GLOBAL_server_lock);

                break;

            case 3:
                //command: 3:
                //Values for the last sampling interval
                pthread_mutex_lock(&EMPI_GLOBAL_server_lock);
                memset(buf, 0, EMPI_COMMBUFFSIZE);

                printf("Statistics: \n");
                char line[1024];
                EMPI_host_type* hostlist = EMPI_GLOBAL_hostlist;
                int index;
                for(index = 0; index < EMPI_GLOBAL_nhosts; index++)
                {
					printf(" ** [%s] rtime is %lld, ptime is %lld, ctime is %f, flops is %lld \n", hostlist->hostname, EMPI_GLOBAL_monitoring_data.rtime, EMPI_GLOBAL_monitoring_data.ptime, EMPI_GLOBAL_monitoring_data.ctime,EMPI_GLOBAL_monitoring_data.flops);					
					printf("\n   %s",line);
                    hostlist=hostlist->next;
                   
                }

                //strncpy(buf_res, line, strlen(line));

                pthread_mutex_unlock(&EMPI_GLOBAL_server_lock);
                break;

            case 4: {
                //command: 4:
                //Statistics subscribe to service
				int active;
				// Poster active
				pthread_mutex_lock(&EMPI_GLOBAL_server_lock);
				active=EMPI_GLOBAL_posteractive;
				pthread_mutex_unlock(&EMPI_GLOBAL_server_lock);
				
				if (strcmp (command.options[i],"on") == 0 && active==0){
					
					pthread_mutex_lock(&EMPI_GLOBAL_server_lock);
					EMPI_GLOBAL_posteractive=1;
					pthread_mutex_unlock(&EMPI_GLOBAL_server_lock);

				  
                    int rc;  // return code
                    pthread_t thread;
                    pthread_attr_t attr;

                    service_arguments args;
                    args.socket = s;
                    args.address = si_other;

                    rc = pthread_attr_init(&attr);
                    check_posix_return(rc, "Initializing attribute");

                    rc = pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
                    check_posix_return(rc, "Setting detached state");

                    rc = pthread_create(&thread, &attr, (void*)&service_poster, (void*)&args);
                    check_posix_return(rc, "Creating thread");
				}
				else{
					pthread_mutex_lock(&EMPI_GLOBAL_server_lock);
					EMPI_GLOBAL_posteractive=0;
					pthread_mutex_unlock(&EMPI_GLOBAL_server_lock);					
				}

                break;
            }
            case 5:

				printf(" Starting the termination of all processes \n" );
				pthread_mutex_lock(&EMPI_GLOBAL_server_lock); // Only the server has an attached thread
				EMPI_GLOBAL_monitoring_data.termination=1;
				pthread_mutex_unlock(&EMPI_GLOBAL_server_lock);
                //MPI_Finalize();
                break;
				
            case 6:  
				// Command 6: triggered execution 
								
				pthread_mutex_lock(&EMPI_GLOBAL_server_lock);
				
				if(EMPI_GLOBAL_listrm[0] ==1){ 
				  printf(" Command ignored: previous command already being processed. Try again later \n" );
				} 
				else{ // Asigna el incremento de procesos (deltaP) a cada clase	
					 i = 0;
					 do
					 {
						for (n = 0; n < EMPI_GLOBAL_nhclasses; n ++) {
							if (strcmp (EMPI_GLOBAL_hclasses[n], command.options[i]) == 0 && atoi(command.options[i+1])!=0) {
								EMPI_GLOBAL_nprocs_class[0][n] = atoi(command.options[i+1]); // Delta proc (increment/decrement in the proc number)
								printf(" Command: Create %d processes in compute node: %s \n",EMPI_GLOBAL_nprocs_class[0][n],EMPI_GLOBAL_hclasses[n]);
								EMPI_GLOBAL_listrm[0] = 1;
							}
						}
						i+=2;
					 } while (command.options[i] != NULL);
				}
				pthread_mutex_unlock(&EMPI_GLOBAL_server_lock);
                break;

			case 7:  
				// Command 7: change the monitoring metrics
								
				len0=strlen(command.options[0])+1;
				len1=strlen(command.options[1])+1;
				if(len0>EMPI_Monitor_string_size-2 || len1>EMPI_Monitor_string_size-2){
					printf(" Command ignored: name of the events is too large  \n" );
				}
				else{
				  pthread_mutex_lock(&EMPI_GLOBAL_server_lock);
				  memcpy(EMPI_GLOBAL_PAPI_nhwpc_1,command.options[0],len0);
				  memcpy(EMPI_GLOBAL_PAPI_nhwpc_2,command.options[1],len1);
				  pthread_mutex_unlock(&EMPI_GLOBAL_server_lock);
				}

                break;
				
			case 8:  
				// Command 8: core binding
								
				pthread_mutex_lock(&EMPI_GLOBAL_server_lock);
				
				// Sets the trigger to do the core binding
				EMPI_GLOBAL_corebinding=1;
				i = 0;
				
				// Creates the binding list per process (n). 
				for (n=0;n<EMPI_max_process;n++) EMPI_GLOBAL_corebindlist[n][0]=0; // First column is the count number
				while (command.options[i] != NULL)
				{
					n=atoi(command.options[i]);
					EMPI_GLOBAL_corebindlist[n][EMPI_GLOBAL_corebindlist[n][0]+1]=atoi(command.options[i+1]);  // Creates the list
					fflush(NULL);
					EMPI_GLOBAL_corebindlist[n][0]++;
					i+=2;
				} 
				
 				pthread_mutex_unlock(&EMPI_GLOBAL_server_lock);
				

                break;
            default:
                break;
        }
       length = sendto(s, buf_res, strlen(buf_res), 0, (struct sockaddr *)&si_other, (socklen_t)slen);
       printf("Sent %d bytes as response\n", length);
    }

    free(buf);
    close(s);
    return 0;
}
