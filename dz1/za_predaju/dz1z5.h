#ifndef _DZ1Z5_H_
#define _DZ1Z5_H_

#include <math.h>
#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ACCURACY 30

#define PI 3.14159265

#define max(x, y) ((x < y) ? y : x)
#define min(x, y) ((x > y) ? y : x)

typedef struct
{
	int numSamples;
	int aquisitionMatrixSize[3];
	int reconstructionMatrixSize[3];
	float kMax[3];
	int gridSize[3];
	float oversample;
	float kernelWidth;
	int binsize;
	int useLUT;
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
int gridding_Gold_Parallel(unsigned int n, parameters params, ReconstructionSample *sample, float *LUT, unsigned int sizeLUT, cmplx *gridData, float *sampleDensity);

#endif