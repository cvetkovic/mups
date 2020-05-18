#!/bin/bash

/usr/local/cuda/bin/nvcc -o dz4z3 dz4z3.cu -lm

./dz4z3 data/small.uks 32