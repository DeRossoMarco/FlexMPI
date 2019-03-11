
#define BUFLEN 512
#define NUMBER_OPTIONS 10
#define EMPI_COMMBUFFSIZE 2048
// WS1 -> Mínimum number of samples to make a prediction
// WS2 -> Maximum number of samples used in the prediction
#define MAX_APPS            50
#define WS1                    2
#define WS2                    5
#define NCLASES             500
#define BUFSIZE             512
#define STAMPSIZE           8  
#define IORATIO             5
#define NUMSAMPLES          1000

// Sampling iterations of the original code 
#define SAMPLING            100
#define reconfig_overhead   15

 struct  applicationset {
    char appname[100];
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
    int numiter;
    };
 
    
extern int    GLOBAL_napps,GLOBAL_SCHEDULING_IO;
extern double cput[NUMSAMPLES][MAX_APPS], tlong[NUMSAMPLES][MAX_APPS];
extern int    niter[NUMSAMPLES][MAX_APPS];
extern int    cnt_app[MAX_APPS];
extern double GLOBAL_IOsize[MAX_APPS];
extern int    GLOBAL_reqIO[MAX_APPS];
extern struct applicationset GLOBAL_app[MAX_APPS];

void IO_scheduler();