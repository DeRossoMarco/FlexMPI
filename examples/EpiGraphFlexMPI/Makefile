MPICC2=$(HOME)/LIBS/mpich/bin/mpicc
MPICC=$(HOME)/LIBS/mpich/bin/mpicc
PAPI=-I$(HOME)/LIBS/papi/include/ -L$(HOME)/LIBS/papi/lib -lpapi -lpfm
GLPK=-I$(HOME)/LIBS/glpk/include/ -L$(HOME)/LIBS/glpk/lib/ -lglpk
SRCEPIFILES=./src/epigraph_mpi.c ./src/epidemiology/epidemiology.c ./src/epidemiology/epidemiology_sync.c
SRCAUXFILES=./src/auxiliar/auxiliar.c
SRCGRAPHFILES=./src/graph_generator/graph_generator.c ./src/gengraph_mpi.c
HDRFILES=-I./src/epidemiology/ -I./src/graph_generator/ -I./src/auxiliar/
CFLAGS=-Wall -g -Wno-unused-variable -Wno-unused-but-set-variable -Wno-misleading-indentation
CSPARSE=./src/csparse/csparse.c -I./src/csparse
MEMCOUNT=./src/memcount/memcount.c -I./src/memcount
LIBXML=-I$(HOME)/LIBS/libxml2 -L$(HOME)/LIBS -lxml2
LIBXMLSTATIC=$(HOME)/LIBS/libxml2.a
LIBGSL=-I$(HOME)/LIBS/gsl/gsl-1.16 -L$(HOME)/LIBS/gsl/gsl-1.16/.libs -L$(HOME)/LIBS/gsl/gsl-1.16/cblas/.libs -lgsl -lgslcblas
LIBGSLSTATIC=$(HOME)/LIBS/gsl/gsl-1.16/.libs/libgsl.a $(HOME)/LIBS/gsl/gsl-1.16/cblas/.libs/libgslcblas.a
LIBEMPI=-I$(HOME)/FlexMPI/include -L$(HOME)/FlexMPI/lib -lempi
HDREMPI=-I$(HOME)/FlexMPI/include
LIBM=-lm
GPROF=-pg
LSTATIC=-lm -lz -lpthread
PREREQUISITES=libxml2-dev libgsl0-dev

all:
	make epigraph
	@echo "+++ MAKE COMPLETE +++"

epigraph:
	@echo "+++ MAKE EPIGRAPH +++"
	$(MPICC) $(CFLAGS) -o epiGraph $(SRCEPIFILES) $(SRCAUXFILES) $(MEMCOUNT) $(HDRFILES) $(CSPARSE) $(LIBXML) $(LIBGSLSTATIC) $(LIBM) $(LIBEMPI) $(PAPI) $(GLPK) $(LIBGSL)

clean:
	make cleanepigraph
	@echo "+++ MAKE CLEAN COMPLETED +++"

cleanepigraph:
	rm -f epiGraph
	@echo "+++ MAKE CLEAN EPIGRAPH COMPLETED +++"
	
