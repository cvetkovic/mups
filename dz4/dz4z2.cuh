#ifndef _DZ4Z2_CUH_
#define _DZ4Z2_CUH_

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <string.h>

#define ACCURACY 0.01

#define MAXLINE 128
#define PIXPERLINE 16

#define TILE_WIDTH 16

#define NUM_OF_GPU_THREADS 1024

const int d = 8;

const double n = (2 * d - 1) * (2 * d - 1);
const double scale = 2.0;

const double constant = scale / n;

char c[MAXLINE];

void pgmsize(char *filename, int *nx, int *ny);
void pgmread(char *filename, void *vp, int nxmax, int nymax, int *nx, int *ny);
void pgmwrite(char *filename, void *vx, int nx, int ny);

double **dosharpen(char *infile, int nx, int ny);
double **dosharpenParallel(char *infile, int nx, int ny, float *timeParallel);
double filter(int d, int i, int j);

int **int2Dmalloc(int nx, int ny);
double **double2Dmalloc(int nx, int ny);

void compareSharp(int w, int h, double **sequential, double **parallel);

double wtime();

#endif