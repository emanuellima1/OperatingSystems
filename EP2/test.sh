#!/bin/bash


rm -f time.out mem.out
for i in {1..30}; do
    a=$(/usr/bin/time -v ./ep2 $1 $2 2>&1 > /dev/null)
    echo "$a" | grep -o -P "(?<=User time \(seconds\)\: )\d*\.?\d*" >> time.out
    echo "$a" | grep -o -P "(?<=Maximum resident set size \(kbytes\)\: )\d*" >> mem.out
done

