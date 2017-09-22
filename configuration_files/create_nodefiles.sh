#!/bin/bash
for i in {1..24}
do
     echo "compute-8-1:$i" > nodefile_c8:$i
done

for j in {1..3}
do
    for i in {1..32}
    do
        echo "compute-9-$j:$i" > nodefile_c9-$j:$i
    done
done
