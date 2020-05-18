#!/bin/bash

/usr/local/cuda/bin/nvcc -o dz4z2 dz4z2.cu -lm

./dz4z2 data_dz4z2/balloons_noisy.pgm
./dz4z2 data_dz4z2/bone_scint.pgm
./dz4z2 data_dz4z2/fuzzy.pgm
./dz4z2 data_dz4z2/lena512.pgm
./dz4z2 data_dz4z2/man.pgm
./dz4z2 data_dz4z2/Rainier_blur.pgm