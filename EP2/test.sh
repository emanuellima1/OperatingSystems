#!/bin/bash


rm -f time.out mem.out
for i in {1..30}; do
    /usr/bin/time -p ./ep2 $1 $2 2>&1 > /dev/null | tail -3 | head -1 | awk '{print $2}'a >> time.out &
    pids[${i}]=$!
    pmap $! | tail -1 | grep -P -o '\d*(?=K)' >> mem.out
done

for pid in ${pids[*]}; do
    wait $pid
done
