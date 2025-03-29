#!/bin/bash
# script takes 4 arguments that are given to the master worker program

gcc master-worker.c -o master-worker -pthread -Wall
./master-worker $1 $2 $3 $4 > output 
awk -f test-master-worker.awk MAX=$1 output
