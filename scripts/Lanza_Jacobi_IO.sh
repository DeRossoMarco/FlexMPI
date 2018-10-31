nodefile=../run/nodefile1.dat
MPIPATH=/usr/bin

# Uncomment for a local installation of the libraries
#MPIPATH=~/LIBS/mpich/bin/

list=`cat  $nodefile |  awk -F: '{print $1}' `
for item in $list; do
  echo Configuring node $item
  rsh $item rm -f /tmp/jacobi_IO$4
  rsh $item cp $HOME/FlexMPI/examples/jacobi_IO /tmp/jacobi_IO$4
done

controllernode=`cat ../controller/controller.dat`

echo $MPIPATH/mpiexec -genvall -f $HOME/FlexMPI/controller/rankfiles/rankfile$4 -np $1 /tmp/jacobi_IO$4 $5 "${10}" 0.00001 $7 $8 $6 $4 $controllernode -cfile ../run/nodefile2.dat -policy-malleability-triggered -lbpolicy-static -ni 20 -ports $2 $3 -controller $controllernode -IOaction $9

$MPIPATH/mpiexec -genvall -f $HOME/FlexMPI/controller/rankfiles/rankfile$4 -np $1 /tmp/jacobi_IO$4 $5 "${10}" 0.00001 $7 $8 $6 $4 $controllernode -cfile ../run/nodefile2.dat -policy-malleability-triggered -lbpolicy-static -ni 20 -ports $2 $3 -controller $controllernode -IOaction $9


