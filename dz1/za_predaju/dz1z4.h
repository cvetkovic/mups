#ifndef _DZ1Z4_H_
#define _DZ1Z4_H_

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <omp.h>
#include <string.h>

#define ACCURACY 0.01

#define MAXLINE 128
#define PIXPERLINE 16

char c[MAXLINE];

void pgmsize(char *filename, int *nx, int *ny);
void pgmread(char *filename, void *vp, int nxmax, int nymax, int *nx, int *ny);
void pgmwrite(char *filename, void *vx, int nx, int ny);

double **dosharpen(char *infile, int nx, int ny);
double **dosharpenParallel(char *infile, int nx, int ny);
double filter(int d, int i, int j);

int **int2Dmalloc(int nx, int ny);
double **double2Dmalloc(int nx, int ny);

void compareSharp(int w, int h, double **sequential, double **parallel);

double wtime();

#endif