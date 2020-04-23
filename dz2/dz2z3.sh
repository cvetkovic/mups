#!/bin/bash

mpicc -o dz2z3 dz2z3.c -lm

mpirun -np 1 ./dz2z3 data_dz2z3/balloons_noisy.pgm
mpirun -np 1 ./dz2z3 data_dz2z3/bone_scint.pgm
mpirun -np 1 ./dz2z3 data_dz2z3/fuzzy.pgm
mpirun -np 1 ./dz2z3 data_dz2z3/lena512.pgm
mpirun -np 1 ./dz2z3 data_dz2z3/man.pgm
mpirun -np 1 ./dz2z3 data_dz2z3/Rainier_blur.pgm

mpirun -np 2 ./dz2z3 data_dz2z3/balloons_noisy.pgm
mpirun -np 2 ./dz2z3 data_dz2z3/bone_scint.pgm
mpirun -np 2 ./dz2z3 data_dz2z3/fuzzy.pgm
mpirun -np 2 ./dz2z3 data_dz2z3/lena512.pgm
mpirun -np 2 ./dz2z3 data_dz2z3/man.pgm
mpirun -np 2 ./dz2z3 data_dz2z3/Rainier_blur.pgm

mpirun -np 4 ./dz2z3 data_dz2z3/balloons_noisy.pgm
mpirun -np 4 ./dz2z3 data_dz2z3/bone_scint.pgm
mpirun -np 4 ./dz2z3 data_dz2z3/fuzzy.pgm
mpirun -np 4 ./dz2z3 data_dz2z3/lena512.pgm
mpirun -np 4 ./dz2z3 data_dz2z3/man.pgm
mpirun -np 4 ./dz2z3 data_dz2z3/Rainier_blur.pgm

mpirun -np 8 ./dz2z3 data_dz2z3/balloons_noisy.pgm
mpirun -np 8 ./dz2z3 data_dz2z3/bone_scint.pgm
mpirun -np 8 ./dz2z3 data_dz2z3/fuzzy.pgm
mpirun -np 8 ./dz2z3 data_dz2z3/lena512.pgm
mpirun -np 8 ./dz2z3 data_dz2z3/man.pgm
mpirun -np 8 ./dz2z3 data_dz2z3/Rainier_blur.pgm