#ifndef _DZ2Z3_H_
#define _DZ2Z3_H_

#include <mpi.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <string.h>

#define N 1
#define ACCURACY 0.01

#define MAXLINE 128
#define PIXPERLINE 16

#define MASTER_RANK 0

char c[MAXLINE];

void pgmsize(char *filename, int *nx, int *ny);
void pgmread(char *filename, void *vp, int nxmax, int nymax, int *nx, int *ny);
void pgmwrite(char *filename, void *vx, int nx, int ny);

double **dosharpen(char *infile, int nx, int ny);
void dosharpenParallel(int nx, int ny, int d, int start, int end, double **convolution, double **fuzzyPadded, double **sharp, double **sharpCropped);
double filter(int d, int i, int j);

int **int2Dmalloc(int nx, int ny);
double **double2Dmalloc(int nx, int ny);

void compareSharp(int w, int h, double **sequential, double **parallel);

double wtime();

#endif