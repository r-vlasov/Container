#!/bin/sh

for pid in $(seq 3 100)
do 
        kill -9 $pid
done
