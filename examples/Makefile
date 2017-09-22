HOME=/home/desingh
LIBEMPI_DIR=$(HOME)/FlexMPI
PAPI_DIR=$(HOME)/LIBS/papi
GLPK_DIR=$(HOME)/LIBS/glpk
MPICC=~/LIBS/mpich/bin/mpicc


PAPI=-I$(PAPI_DIR)/include/ -L/usr/lib -lpapi -L$(PAPI_DIR)/lib -lpfm
GLPK=-I$(GLPK_DIR)/include/ -L/usr/lib/ -L$(GLPK_DIR)/lib/ -lglpk 
EMPI=-I$(LIBEMPI_DIR)/include/  -L/usr/lib/ -L$(LIBEMPI_DIR)/lib/ -I$(LIBEMPI_DIR)/include -lempi -lmpi 
CFLAGS=-Wall -g
MATH= -lm

.PHONY: jacobi gradient
	
jacobi:

	$(MPICC) $(CFLAGS) -o jacobi_IO_V4.o jacobi_IO_V4.c -c $(EMPI) $(PAPI) $(GLPK) 
	$(MPICC) $(CFLAGS)  jacobi_IO_V4.o -o jacobi_IO $(EMPI) $(PAPI) $(GLPK) 
	@echo "+++MAKE JACOBI COMPLETE+++"
