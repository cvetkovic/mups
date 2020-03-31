#!/bin/bash

gcc -o dz1z4 dz1z4.c -lm

export OMP_NUM_THREADS=1

./dz1z4 dz1z4/balloons_noisy.pgm
./dz1z4 dz1z4/bone_scint.pgm
./dz1z4 dz1z4/fuzzy.pgm
./dz1z4 dz1z4/lena512.pgm
./dz1z4 dz1z4/man.pgm
./dz1z4 dz1z4/Rainier_blur.pgm

export OMP_NUM_THREADS=2

./dz1z4 dz1z4/balloons_noisy.pgm
./dz1z4 dz1z4/bone_scint.pgm
./dz1z4 dz1z4/fuzzy.pgm
./dz1z4 dz1z4/lena512.pgm
./dz1z4 dz1z4/man.pgm
./dz1z4 dz1z4/Rainier_blur.pgm

export OMP_NUM_THREADS=4

./dz1z4 dz1z4/balloons_noisy.pgm
./dz1z4 dz1z4/bone_scint.pgm
./dz1z4 dz1z4/fuzzy.pgm
./dz1z4 dz1z4/lena512.pgm
./dz1z4 dz1z4/man.pgm
./dz1z4 dz1z4/Rainier_blur.pgm

export OMP_NUM_THREADS=8

./dz1z4 dz1z4/balloons_noisy.pgm
./dz1z4 dz1z4/bone_scint.pgm
./dz1z4 dz1z4/fuzzy.pgm
./dz1z4 dz1z4/lena512.pgm
./dz1z4 dz1z4/man.pgm
./dz1z4 dz1z4/Rainier_blur.pgm