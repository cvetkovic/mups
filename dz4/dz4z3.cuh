#ifndef _DZ4Z3_CUH_
#define _DZ4Z3_CUH_

#include <math.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ACCURACY   1
#define DEVIATION 30

#define PI 3.14159265

#define NUM_OF_GPU_THREADS 1024

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

void gridding_Gold(unsigned int n, parameters params, ReconstructionSample *sample, float *LUT, unsigned int sizeLUT, cmplx *gridData, float *sampleDensity);
void gridding_Parallel(unsigned int n, parameters *params, ReconstructionSample *sample, float *LUT, unsigned int sizeLUT, cmplx *gridData, float *sampleDensity, float *timeParallel);

#endif