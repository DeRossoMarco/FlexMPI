# README #

FLEX-MPI is a runtime system that extends the functionalities of the MPI library by providing dynamic load balancing and performance-aware malleability capabilities to MPI applications. Dynamic load balancing allows FLEX-MPI to adapt the workload assignments at runtime to the performance of the computing elements that execute the parallel application. Performance-aware malleability improves the performance of applications by changing the number of processes at runtime.

The project goals are:
 
* To provide MPI applications of malleable capabilities that permit to specify a given application execution time. FLEX-MPI will automatically create or remove new processes to adjust the execution time to the desired objective.

* FLEX-MPI includes two polices that can be used in combination with the malleable capabilities. The aim is to minimize the operational cost or the application energy consumption by efficiently scheduling the application proccesses based on the cluster characteristics. Flex-MPI supports CPU-based heterogeneous multicore clusters.

* Flex-MPI performs all the optimizations during the application execution, in a transparent way, without user intervention.
 

## Requirements ##

The following libraries and compiler are required for compilation:

* GCC (>= 4.9)
* PAPI (>= 5.5.1.0)
* GLPK (GNU Linear Programming Kit) (>= 4.47)
* MPI MPICH distribution (>3.2)

## How do I get set up? ##

```bash
git clone https://github.com/arcosuc3m/flexmpi

``` Assumming that FlexMPI is installed in ~/FlexMPI 
```bash

cd ~/FlexMPI
[configure Makefile providing paths to the exisitng libraries and include files]
make

cd ~/FlexMPI/examples
[configure Makefile providing paths to the exisitng libraries and include files]
make

cd ~/FlexMPI/controller
[configure Makefile providing paths to the exisitng libraries and include files]
make

cd ~/FlexMPI/scripts
chmod 755 ./Lanza_Jacobi_IO.sh

```Environment variables
Assumming that the libraries are installed in $HOME/LIBS
export LD_LIBRARY_PATH=$HOME/LIBS/glpk/lib/:$HOME/FlexMPI/lib/:$HOME/LIBS/mpich/lib/:$HOME/LIBS/papi/lib/:$LD_LIBRARY_PATH

```Configuration files
It is necessary to setup the configuration files in ~/FlexMPI/configuration_files directory

## Execution ##

```bash. 
cd ~/FlexMPI/controller
./controller


## Acknowledgements ##


This work has been partially supported by the Spanish Ministry of Economy and Competitiveness 
under the project TIN2013-41350-P (Scalable Data Management Techniques for High-End Computing Systems)



## References ##


* Gonzalo Martin, David E. Singh, Maria-Cristina Marinescu and Jesus Carretero. Enhancing the performance of malleable MPI applications by using performance-aware dynamic reconfiguration. Parallel Computing. Vol. 46, No. 0. Pages: 60-77. 2015.

* Manuel Rodríguez-Gonzalo, David E. Singh, Javier García Blas and Jesús Carretero. Improving the energy efficiency of MPI applications by means of malleability. 24th Euromicro International Conference on Parallel, Distributed and Network-based Processing (PDP). Heraklion, Greece, February, 2016.

* Gonzalo Martín, David E. Singh, Maria-Cristina Marinescu and Jesús Carretero. FLEX-MPI: an MPI extension for supporting dynamic load balancing on heterogeneous non-dedicated systems. European Conference on Parallel Computing (EUROPAR). Aachen, Germany. 2013.

* Gonzalo Martín, David E. Singh, Maria-Cristina Marinescu and Jesús Carretero. Runtime support for adaptive resource provisioning in MPI applications. The 19th European MPI Users’ Group Meeting – EuroMPI. Vienna, Austria. 2012.
