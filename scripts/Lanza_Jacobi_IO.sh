nodefile=../run/nodefile1.dat


list=`cat  $nodefile |  awk -F: '{print $1}' `
for item in $list; do
  echo Configuring node $item
  rsh $item rm -f /tmp/jacobi_IO$4
  rsh $item cp /home/desingh/FlexMPI/examples/jacobi_IO /tmp/jacobi_IO$4
done

controllernode=`cat ../controller/controller.dat`

echo ~/LIBS/mpich/bin/mpiexec -genvall -f /home/desingh/FlexMPI/controller/rankfiles/rankfile$4 -np $1 /tmp/jacobi_IO$4 $5 $10 0.00001 $7 $8 $6 $4 $controllernode -cfile ../run/nodefile2.dat -policy-malleability-triggered -lbpolicy-static -ni 20 -ports $2 $3 -controller $controllernode -IOaction $9

~/LIBS/mpich/bin/mpiexec -genvall -f /home/desingh/FlexMPI/controller/rankfiles/rankfile$4 -np $1 /tmp/jacobi_IO$4 $5 $10 0.00001 $7 $8 $6 $4 $controllernode -cfile ../run/nodefile2.dat -policy-malleability-triggered -lbpolicy-static -ni 20 -ports $2 $3 -controller $controllernode -IOaction $9


