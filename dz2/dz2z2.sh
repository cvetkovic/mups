#!/bin/bash

mpicc -o dz2z2 dz2z2.c -lm

mpirun -np 2 ./dz2z2 500 500 200
mpirun -np 2 ./dz2z2 500 500 500
mpirun -np 2 ./dz2z2 500 500 1000
mpirun -np 2 ./dz2z2 1000 1000 200
mpirun -np 2 ./dz2z2 1000 1000 500
mpirun -np 2 ./dz2z2 1000 1000 1000
mpirun -np 2 ./dz2z2 2000 1000 200
mpirun -np 2 ./dz2z2 2000 1000 500
mpirun -np 2 ./dz2z2 2000 1000 1000

mpirun -np 3 ./dz2z2 500 500 200
mpirun -np 3 ./dz2z2 500 500 500
mpirun -np 3 ./dz2z2 500 500 1000
mpirun -np 3 ./dz2z2 1000 1000 200
mpirun -np 3 ./dz2z2 1000 1000 500
mpirun -np 3 ./dz2z2 1000 1000 1000
mpirun -np 3 ./dz2z2 2000 1000 200
mpirun -np 3 ./dz2z2 2000 1000 500
mpirun -np 3 ./dz2z2 2000 1000 1000

mpirun -np 5 ./dz2z2 500 500 200
mpirun -np 5 ./dz2z2 500 500 500
mpirun -np 5 ./dz2z2 500 500 1000
mpirun -np 5 ./dz2z2 1000 1000 200
mpirun -np 5 ./dz2z2 1000 1000 500
mpirun -np 5 ./dz2z2 1000 1000 1000
mpirun -np 5 ./dz2z2 2000 1000 200
mpirun -np 5 ./dz2z2 2000 1000 500
mpirun -np 5 ./dz2z2 2000 1000 1000

mpirun -np 9 ./dz2z2 500 500 200
mpirun -np 9 ./dz2z2 500 500 500
mpirun -np 9 ./dz2z2 500 500 1000
mpirun -np 9 ./dz2z2 1000 1000 200
mpirun -np 9 ./dz2z2 1000 1000 500
mpirun -np 9 ./dz2z2 1000 1000 1000
mpirun -np 9 ./dz2z2 2000 1000 200
mpirun -np 9 ./dz2z2 2000 1000 500
mpirun -np 9 ./dz2z2 2000 1000 1000