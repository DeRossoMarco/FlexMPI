/**
* @version        FlexMPI v3.1
* @copyright    Copyright (C) 2018 Universidad Carlos III de Madrid. All rights reserved.
* @license        GNU/GPL, see LICENSE.txt
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
 *                                                                                                                                        *
 *    FLEX-MPI                                                                                                                            *
 *                                                                                                                                        *
 *    File:       vars.h                                                                                                                    *
 *                                                                                                                                        *
 ****************************************************************************************************************************************/

#ifndef _EMPI_VARS_H_
#define _EMPI_VARS_H_
#ifdef VARIABLES
#define EXTERN
#else
#define EXTERN extern
#endif

    
/* include */
#include <types.h>

/* global constants */
static const int EMPI_MAX_FILENAME         = 1024;
static const int EMPI_MAX_LINE_LENGTH     = 32768;
static const int EMPI_MAX_NPROCS         = 24;

static const int EMPI_root                = 0;

static const int EMPI_DISJOINT            = 674782;
static const int EMPI_SHARED            = 617828;

static const int EMPI_CBAL                = 182919;
static const int EMPI_CUSER                = 192018;

static const int EMPI_TRUE                 = 674321;
static const int EMPI_FALSE                = 684312;

static const int EMPI_ERROR                = 939199;

static const int EMPI_STRUCT_DENSE        = 534321;
static const int EMPI_STRUCT_SPARSE        = 544312;

static const int EMPI_STATIC             = 324542;
static const int EMPI_DYNAMIC             = 325765;

static const int EMPI_NULL                = 949998;

static const int EMPI_END                 = 921028;

static const int EMPI_ACTIVE            = 776765;
static const int EMPI_REMOVED            = 787634;
static const int EMPI_TO_REMOVE            = 792214;

static const int EMPI_SPAWNED            = 938594;
static const int EMPI_NATIVE            = 941490;

static const int EMPI_LBMFLOPS            = 854392;
static const int EMPI_LBCOUNTS            = 829010;
static const int EMPI_LBSTATIC            = 829099;

static const int EMPI_ROWS                = 748838;
static const int EMPI_NNZ                 = 782891;
static const int EMPI_FCOST             = 728191;

static const int EMPI_BALANCED             = 189312;
static const int EMPI_UNBALANCED        = 135894;

static const int EMPI_BURST             = 225615;
static const int EMPI_LONG_TERM            = 234684;

static const int EMPI_AVAIL_NODE        = 678292;
static const int EMPI_OCCUP_NODE        = 627543;

static const int EMPI_DEDICATED            = 689894;
static const int EMPI_NON_DEDICATED        = 685138;

static const int EMPI_TREAL                = 896646;
static const int EMPI_TCPU                = 877789;

static const int EMPI_ADAPTABILITY_EX    = 743289;
static const int EMPI_ADAPTABILITY_LP    = 718291;
// Communication variables
static const int EMPI_COMMBUFFSIZE        = 2048;
static const int EMPI_COMMNPACK            = 10;
static const int EMPI_COMMNUMOPTIONS     = 10;


// Communication variables

EXTERN int EMPI_GLOBAL_recvport;
EXTERN int EMPI_GLOBAL_sendport;


/* global variables */
EXTERN int EMPI_GLOBAL_nhosts;

EXTERN double EMPI_GLOBAL_percentage;

EXTERN int EMPI_GLOBAL_lbalance;

EXTERN int EMPI_GLOBAL_maxprocs;
EXTERN int EMPI_GLOBAL_minprocs;

EXTERN double EMPI_GLOBAL_comm_prev;

EXTERN double EMPI_GLOBAL_over_si;

EXTERN int EMPI_GLOBAL_niter;
EXTERN int EMPI_GLOBAL_niter_lb;

EXTERN int EMPI_GLOBAL_wpolicy; //workload policy

EXTERN int EMPI_GLOBAL_lbpolicy; //load balancing policy

EXTERN int *EMPI_GLOBAL_vcounts;
EXTERN int *EMPI_GLOBAL_displs;

EXTERN int EMPI_GLOBAL_capture_comms;

EXTERN int EMPI_GLOBAL_mpolicy; //monitor policy

EXTERN int EMPI_GLOBAL_concurrency;

EXTERN int EMPI_GLOBAL_hostid; //host id where the process is being executed

EXTERN int EMPI_GLOBAL_nhclasses; //number of host classes

EXTERN char EMPI_GLOBAL_hclasses[100][128]; //host classes

EXTERN char EMPI_GLOBAL_controller[512];   // external server name
EXTERN char EMPI_GLOBAL_application[512];  // application name

EXTERN int EMPI_GLOBAL_lbalance_disabled;

EXTERN int EMPI_GLOBAL_perform_load_balance;

//Malleability
EXTERN int EMPI_GLOBAL_nextrm;
EXTERN int EMPI_GLOBAL_listrm[100];
EXTERN int EMPI_GLOBAL_nprocs_class[100][10];

EXTERN int EMPI_GLOBAL_initnc;

EXTERN int EMPI_GLOBAL_status; //active or removed status

EXTERN int EMPI_GLOBAL_type; //spawned or native

EXTERN int EMPI_GLOBAL_allocation;

EXTERN double EMPI_GLOBAL_obj_texec;
EXTERN double EMPI_GLOBAL_cum_cost;
EXTERN double EMPI_GLOBAL_cum_time;
EXTERN double EMPI_GLOBAL_obj_texec_threshold;

EXTERN double EMPI_GLOBAL_spawn_cost;
EXTERN double EMPI_GLOBAL_remove_cost;

EXTERN int EMPI_GLOBAL_spolicy; //spawn policy: available nodes or occupied nodes

EXTERN int EMPI_GLOBAL_iteration;

EXTERN double EMPI_GLOBAL_tcomp;   //aggregated computation time
EXTERN double EMPI_GLOBAL_tcomm;   //aggregated communication time
EXTERN double EMPI_GLOBAL_tover;   //aggregated overhead time of the monitor functionality
EXTERN double EMPI_GLOBAL_tio;     //aggregated communication time

EXTERN double EMPI_GLOBAL_tcomm_itinit;
EXTERN double EMPI_GLOBAL_tcomm_interval;

EXTERN double EMPI_GLOBAL_tio_itinit;
EXTERN double EMPI_GLOBAL_tio_interval;

EXTERN double EMPI_GLOBAL_tcomp_ini;
EXTERN double EMPI_GLOBAL_tcomp_fin;

EXTERN double EMPI_GLOBAL_tcomm_ini;
EXTERN double EMPI_GLOBAL_tcomm_fin;

EXTERN double EMPI_GLOBAL_tio_last;    // Timestamp of the previous I/O operation
EXTERN double EMPI_GLOBAL_tio_ini;     // Timestamp before the I/O operation
EXTERN double EMPI_GLOBAL_tio_fin;     // Timestamp after the I/O operation
EXTERN int    EMPI_GLOBAL_socket;      // Socket for sending control data
EXTERN struct sockaddr_in EMPI_GLOBAL_controller_addr;  // Address of the controller 
EXTERN double EMPI_GLOBAL_dummyIO;     // When <0 performs MPI I/O; When >=0 performs dummy I/O of EMPI_GLOBAL_dummyIO seconds 

EXTERN double EMPI_GLOBAL_iterative_ini;
EXTERN double EMPI_GLOBAL_iterative_end;

EXTERN long long EMPI_GLOBAL_tover_ini;

EXTERN double EMPI_GLOBAL_threshold;

EXTERN double EMPI_GLOBAL_Load_threshold;

EXTERN int EMPI_GLOBAL_self_adaptation;

//FIXME
EXTERN double EMPI_GLOBAL_alpha;
EXTERN double EMPI_GLOBAL_beta;
EXTERN double EMPI_GLOBAL_bandwidth;
EXTERN double EMPI_GLOBAL_gamma;

EXTERN float EMPI_GLOBAL_sampling_time;

EXTERN int EMPI_GLOBAL_hsteps;
EXTERN int *EMPI_GLOBAL_hmon;
EXTERN int EMPI_GLOBAL_hpos;

EXTERN int EMPI_GLOBAL_nc;

EXTERN int EMPI_GLOBAL_PAPI_init;
EXTERN long long EMPI_GLOBAL_PAPI_rtime;
EXTERN long long EMPI_GLOBAL_PAPI_rtime_init;
EXTERN long long EMPI_GLOBAL_PAPI_ptime;
EXTERN long long EMPI_GLOBAL_PAPI_ptime_init;
EXTERN long long EMPI_GLOBAL_PAPI_flops;
EXTERN long long EMPI_GLOBAL_PAPI_hwpc_1;
EXTERN long long EMPI_GLOBAL_PAPI_hwpc_2;
EXTERN char EMPI_GLOBAL_PAPI_nhwpc_1[EMPI_Monitor_string_size];
EXTERN char EMPI_GLOBAL_PAPI_nhwpc_2[EMPI_Monitor_string_size];

EXTERN int EMPI_GLOBAL_PAPI_eventSet;

EXTERN int EMPI_GLOBAL_corebinding;

EXTERN int EMPI_GLOBAL_delayio;
EXTERN double EMPI_GLOBAL_delayiotime;

EXTERN long long EMPI_GLOBAL_PAPI_rtime_lb;
EXTERN long long EMPI_GLOBAL_PAPI_ptime_lb;
EXTERN long long EMPI_GLOBAL_PAPI_flops_lb;
EXTERN long long EMPI_GLOBAL_PAPI_hwpc_1_lb;
EXTERN long long EMPI_GLOBAL_PAPI_hwpc_2_lb;
EXTERN double EMPI_GLOBAL_tcomm_interval_lb;

EXTERN double EMPI_GLOBAL_overhead_rpolicy;
EXTERN double EMPI_GLOBAL_overhead_lbalance;
EXTERN double EMPI_GLOBAL_overhead_processes;         // accumulated overhead of process creation/destruction
EXTERN double EMPI_GLOBAL_lastoverhead_processes;  // last overhead of the last operation of process creation/destruction
EXTERN double EMPI_GLOBAL_overhead_rdata;             // accumulated overhead of data redistribution
EXTERN double EMPI_GLOBAL_lastoverhead_rdata;         // last overhead of data redistribution
EXTERN double EMPI_GLOBAL_overhead_aux;

EXTERN long long *EMPI_GLOBAL_track_flops;

EXTERN long long *EMPI_GLOBAL_track_rtime;

EXTERN int EMPI_GLOBAL_Adaptability_policy;

EXTERN int EMPI_GLOBAL_PAPI_numevents;



EXTERN double EMPI_GLOBAL_ENERGY_aggregated_final;
EXTERN double EMPI_GLOBAL_ENERGY_aggregated_start;
EXTERN double EMPI_GLOBAL_ENERGY_aggregated_init;
EXTERN double EMPI_GLOBAL_ENERGY_aggregated_end;
EXTERN double EMPI_GLOBAL_ENERGY_aggregated_elapsed;
EXTERN double EMPI_GLOBAL_ENERGY_aggregated_elapsed_final;
EXTERN double EMPI_GLOBAL_ENERGY_aggregated_power;
EXTERN double EMPI_GLOBAL_ENERGY_aggregated_power_final;

EXTERN double * EMPI_GLOBAL_power_monitoring_data;

EXTERN double EMPI_GLOBAL_ENERGY_monitoring_temp[4];

EXTERN int EMPI_GLOBAL_debug_comms;
EXTERN int EMPI_GLOBAL_energy_rank;
EXTERN int EMPI_GLOBAL_nhosts_aux;
EXTERN int EMPI_GLOBAL_energy_eficiency_op_mode;
EXTERN int EMPI_GLOBAL_non_exclusive;

EXTERN long long EMPI_GLOBAL_PAPI_flops_iteration;
EXTERN long long EMPI_GLOBAL_PAPI_it_time;
EXTERN long long EMPI_GLOBAL_PAPI_real_flops_iteration;

EXTERN long long t1;
EXTERN long long t2;
EXTERN int flag;

EXTERN int EMPI_GLOBAL_flag_dynamic;

EXTERN int EMPI_GLOBAL_flag_enter;

EXTERN int EMPI_GLOBAL_PAPI_eventSet_energy;
EXTERN char event_names[MAX_RAPL_EVENTS][PAPI_MAX_STR_LEN];
EXTERN char units[MAX_RAPL_EVENTS][PAPI_MIN_STR_LEN];
EXTERN MPI_Comm EMPI_GLOBAL_comm_energy;

EXTERN MPI_Comm EMPI_COMM_WORLD; //EMPI Global communicator

EXTERN EMPI_Cost_type *EMPI_GLOBAL_cost;

EXTERN EMPI_Data_type *EMPI_GLOBAL_Data; //register shared data structure

EXTERN EMPI_host_type *EMPI_GLOBAL_hostlist; //physical resources

EXTERN MPI_Datatype EMPI_Monitor_Datatype;

EXTERN EMPI_Monitor_type EMPI_GLOBAL_monitor;

EXTERN EMPI_Comm_type* EMPI_GLOBAL_comms;

EXTERN EMPI_Class_type* EMPI_GLOBAL_system_classes;

EXTERN char EMPI_GLOBAL_host_name[MPI_MAX_PROCESSOR_NAME];

EXTERN EMPI_Spawn_data EMPI_GLOBAL_spawn_data;

EXTERN EMPI_Monitor_type EMPI_GLOBAL_monitoring_data;

EXTERN pthread_mutex_t EMPI_GLOBAL_server_lock;

// Core binding
EXTERN int EMPI_GLOBAL_corebindlist[EMPI_max_process][32];

// Poster thread active
EXTERN int EMPI_GLOBAL_posteractive;

// Large-array allocation
EXTERN int EMPI_array_alloc;

#endif
