nodefile=nodefile_c11x
HOME=/home/desingh

list=`cat  $HOME/FlexMPI/configuration_files/nodefiles/$nodefile |  awk -F: '{print $1}' `
for item in $list; do
  rsh $item cp $HOME/FlexMPI/examples/jacobi_IO /tmp/jacobi_IO$4
done

$HOME/LIBS/mpich/bin/mpiexec -genvall -f $HOME/FlexMPI/controller/rankfiles/rankfile$4 -np $1 /tmp/jacobi_IO$4 500 10000 0.00001 100 1 70 -cfile $HOME/FlexMPI/configuration_files/corefiles/corefile_energy_c11x -policy-malleability-triggered -lbpolicy-counts 15000 100 -ni 100 -ports $2 $3


