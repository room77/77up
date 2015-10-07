#!/bin/bash -e

../../auto_build --opt thread_bench

rep=10
tot=0
for i in `seq 1 $rep` ; do
tot=$(echo $tot + `../../auto_build --opt --run thread_bench | awk '/Processed/{print $5}'` | bc -l)
echo -n .
done

echo -e "\naverage time: `echo "scale=2; $tot / $rep " | bc -l` ms"
