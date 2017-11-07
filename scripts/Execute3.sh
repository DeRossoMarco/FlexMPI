#!/bin/bash

if [ "$#" -ne 4 ]; then
    echo "Illegal number of parameters"
fi

list=`cat $HOME/FlexMPI/configuration_files/nodefiles/nodefile.demo2 |  awk -F: '{print $1}'`

for item in $list; do
 	rsh $item rm -f /tmp/jacobi_IO$4 
	rsh $item cp $HOME/FlexMPI/examples/jacobi_IO /tmp/jacobi_IO$4
done


$HOME/LIBS/mpich/bin/mpiexec -genvall -f $HOME/FlexMPI/controller/rankfiles/rankfile1.demo2 -np $1 /tmp/jacobi_IO$4 500 10000 0.00001 100 1 70 -cfile $HOME/FlexMPI/configuration_files/corefiles/corefile.demo2 -policy-malleability-triggered -lbpolicy-counts 15000 100 -ni 100 -ports $2 $3


