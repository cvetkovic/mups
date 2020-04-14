#ifndef _DZ2Z4_H_
#define _DZ2Z4_H_

#include <mpi.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define N 4
#define ACCURACY 30

#define PI 3.14159265

#define MASTER_RANK 0

#define max(x, y) ((x < y) ? y : x)
#define min(x, y) ((x > y) ? y : x)

typedef struct
{
	int numSamples;
	int aquisitionMatrixSize[3];
	int reconstructionMatrixSize[3];
	int gridSize[3];
	int binsize;
	int useLUT;
	float kMax[3];
	float oversample;
	float kernelWidth;
} parameters;

typedef struct
{
	float real;
	float imag;
	float kX;
	float kY;
	float kZ;
	float sdc;
} ReconstructionSample;

typedef struct
{
	float real;
	float imag;
} cmplx;

void calculateLUT(float beta, float width, float **LUT, unsigned int *sizeLUT);

int gridding_Gold(unsigned int n, parameters params, ReconstructionSample *sample, float *LUT, unsigned int sizeLUT, cmplx *gridData, float *sampleDensity);
int gridding_Gold_Parallel(int start, int end, parameters params, ReconstructionSample *sample, float *LUT, unsigned int sizeLUT, cmplx *gridData, float *sampleDensity);

#endif