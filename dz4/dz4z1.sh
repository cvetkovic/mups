#!/bin/bash

/usr/local/cuda/bin/nvcc -lm -o julia julia.cu 

./julia 500 500 200
./julia 500 500 500
./julia 500 500 1000
./julia 1000 1000 200
./julia 1000 1000 500
./julia 1000 1000 1000
./julia 2000 1000 200
./julia 2000 1000 500
./julia 2000 1000 1000