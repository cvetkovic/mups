#!/bin/bash

gcc -fopenmp -o julia julia.c
./julia
