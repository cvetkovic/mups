#!/bin/bash

gcc -fopenmp -O2 main.c CPU_kernels.c -o mri-gridding -lm
./run