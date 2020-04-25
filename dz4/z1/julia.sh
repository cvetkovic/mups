#!/bin/bash

nvcc -lm -o julia julia.cu 
./julia
