#!/bin/bash

mpicc -o dz2z4 dz2z4.c -lm

mpirun -np 4 ./dz2z4 data_dz2z4/small.uks 32