#!/bin/bash

make clean
make

echo 1 THREAD
export OMP_NUM_THREADS=1
./run

echo 2 THREADS
export OMP_NUM_THREADS=2
./run

echo 4 THREADS
export OMP_NUM_THREADS=4
./run

echo 8 THREADS
export OMP_NUM_THREADS=8
./run