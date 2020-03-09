#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "UDTypes.h"
#include "CPU_kernels.h"

#include <omp.h>
#define ACCURACY 0.01

#define PI 3.14159265

/************************************************************ 
 * This function reads the parameters from the file provided
 * as a comman line argument.
 ************************************************************/
void setParameters(FILE* file, parameters* p){
  fscanf(file,"aquisition.numsamples=%d\n",&(p->numSamples));
  fscanf(file,"aquisition.kmax=%f %f %f\n",&(p->kMax[0]), &(p->kMax[1]), &(p->kMax[2]));
  fscanf(file,"aquisition.matrixSize=%d %d %d\n", &(p->aquisitionMatrixSize[0]), &(p->aquisitionMatrixSize[1]), &(p->aquisitionMatrixSize[2]));
  fscanf(file,"reconstruction.matrixSize=%d %d %d\n", &(p->reconstructionMatrixSize[0]), &(p->reconstructionMatrixSize[1]), &(p->reconstructionMatrixSize[2]));
  fscanf(file,"gridding.matrixSize=%d %d %d\n", &(p->gridSize[0]), &(p->gridSize[1]), &(p->gridSize[2]));
  fscanf(file,"gridding.oversampling=%f\n", &(p->oversample));
  fscanf(file,"kernel.width=%f\n", &(p->kernelWidth));
  fscanf(file,"kernel.useLUT=%d\n", &(p->useLUT));

  printf("  Number of samples = %d\n", p->numSamples);
  printf("  Grid Size = %dx%dx%d\n", p->gridSize[0], p->gridSize[1], p->gridSize[2]);
  printf("  Input Matrix Size = %dx%dx%d\n", p->aquisitionMatrixSize[0], p->aquisitionMatrixSize[1], p->aquisitionMatrixSize[2]);
  printf("  Recon Matrix Size = %dx%dx%d\n", p->reconstructionMatrixSize[0], p->reconstructionMatrixSize[1], p->reconstructionMatrixSize[2]);
  printf("  Kernel Width = %f\n", p->kernelWidth);
  printf("  KMax = %.2f %.2f %.2f\n", p->kMax[0], p->kMax[1], p->kMax[2]);
  printf("  Oversampling = %f\n", p->oversample);
  printf("  GPU Binsize = %d\n", p->binsize);
  printf("  Use LUT = %s\n", (p->useLUT)?"Yes":"No");
}

/************************************************************ 
 * This function reads the sample point data from the kspace
 * and klocation files (and sdc file if provided) into the
 * sample array.
 * Returns the number of samples read successfully.
 ************************************************************/
unsigned int readSampleData(parameters params, FILE* uksdata_f, ReconstructionSample* samples){
  unsigned int i;

  for(i=0; i<params.numSamples; i++){
    if (feof(uksdata_f)){
      break;
    }
    fread((void*) &(samples[i]), sizeof(ReconstructionSample), 1, uksdata_f);
  }

  float kScale[3];
  kScale[0] = (float)(params.aquisitionMatrixSize[0])/((float)(params.reconstructionMatrixSize[0])*(float)(params.kMax[0]));
  kScale[1] = (float)(params.aquisitionMatrixSize[1])/((float)(params.reconstructionMatrixSize[1])*(float)(params.kMax[1]));
  kScale[2] = (float)(params.aquisitionMatrixSize[2])/((float)(params.reconstructionMatrixSize[2])*(float)(params.kMax[2]));

  int size_x = params.gridSize[0];
  int size_y = params.gridSize[1];
  int size_z = params.gridSize[2];

  float ax = (kScale[0]*(size_x-1))/2.0;
  float bx = (float)(size_x-1)/2.0;

  float ay = (kScale[1]*(size_y-1))/2.0;
  float by = (float)(size_y-1)/2.0;

  float az = (kScale[2]*(size_z-1))/2.0;
  float bz = (float)(size_z-1)/2.0;

  int n;
  for(n=0; n<i; n++){
    samples[n].kX = floor((samples[n].kX*ax)+bx);
    samples[n].kY = floor((samples[n].kY*ay)+by);
    samples[n].kZ = floor((samples[n].kZ*az)+bz);
  }

  return i;
}

void compareOutput(cmplx* gridData, cmplx* gridDataParallel, float* sampleDensity, float* sampleDensityParallel)
{
	if (fabs(gridData->real - gridDataParallel->real) > ACCURACY || 
      fabs(gridData->imag - gridDataParallel->imag) > ACCURACY || 
      fabs(*sampleDensity - *sampleDensityParallel) > ACCURACY)
		printf("Test FAILED\n");
	else
		printf("Test PASSED\n");
}

void main (int argc, char* argv[]){
  
  char uksfile[256];
  char uksdata[256];
  parameters params;

  FILE* uksfile_f = NULL;
  FILE* uksdata_f = NULL;

  if (argc != 3) return;
  
  strcpy(uksfile, argv[1]);
  strcpy(uksdata, argv[1]);
  strcat(uksdata,".data");

  uksfile_f = fopen(uksfile,"r");
  if (uksfile_f == NULL){
    printf("ERROR: Could not open %s\n", uksfile);
    exit(1);
  }

  printf("\nReading parameters\n");

  if (argc >= 2){
    params.binsize = atoi(argv[2]);
  } else { //default binsize value;
    params.binsize = 128;
  }

  setParameters(uksfile_f, &params);

  ReconstructionSample* samples = (ReconstructionSample*) malloc (params.numSamples*sizeof(ReconstructionSample)); 
  float* LUT, *LUTParallel; 
  unsigned int sizeLUT, sizeLUTParallel; 

  int gridNumElems = params.gridSize[0]*params.gridSize[1]*params.gridSize[2];

  cmplx* gridData = (cmplx*) calloc (gridNumElems, sizeof(cmplx)); 
  float* sampleDensity = (float*) calloc (gridNumElems, sizeof(float)); 
  cmplx* gridDataParallel = (cmplx*) calloc (gridNumElems, sizeof(cmplx)); 
  float* sampleDensityParallel = (float*) calloc (gridNumElems, sizeof(float)); 

  if (samples == NULL){
    printf("ERROR: Unable to allocate memory for input data\n");
    exit(1);
  }

  if (sampleDensity == NULL || gridData == NULL || sampleDensityParallel == NULL || gridDataParallel){
    printf("ERROR: Unable to allocate memory for output data\n");
    exit(1);
  }

  uksdata_f = fopen(uksdata,"rb");

  if(uksdata_f == NULL){
    printf("ERROR: Could not open data file\n");
    exit(1);
  }

  printf("Reading input data from files\n");

  unsigned int n = readSampleData(params, uksdata_f, samples);
  fclose(uksdata_f);

  double timeSequential = 0, timeParallel = 0;

  if (params.useLUT){
    printf("Generating Look-Up Table\n");
    float beta = PI * sqrt(4*params.kernelWidth*params.kernelWidth/(params.oversample*params.oversample) * (params.oversample-.5)*(params.oversample-.5)-.8);
    
    timeSequential = omp_get_wtime();
    calculateLUT(beta, params.kernelWidth, &LUT, &sizeLUT);
    timeSequential = omp_get_wtime() - timeSequential;

    timeParallel = omp_get_wtime();
    calculateLUT(beta, params.kernelWidth, &LUTParallel, &sizeLUTParallel);
    timeParallel = omp_get_wtime() - timeParallel;
  }

  timeSequential += omp_get_wtime();
  gridding_Gold(n, params, samples, LUT, sizeLUT, gridData, sampleDensity);
  timeSequential = omp_get_wtime() - timeSequential;

  timeParallel += omp_get_wtime();
  gridding_Gold_parallel(n, params, samples, LUTParallel, sizeLUTParallel, gridDataParallel, sampleDensityParallel);
  timeParallel = omp_get_wtime() - timeParallel;

  compareOutput(gridData, gridDataParallel, sampleDensity, sampleDensityParallel);

  printf("Sequential time: %f\n", timeSequential);
  printf("Parallel time: %f\n", timeParallel);

  if (params.useLUT){
    free(LUT);
    free(LUTParallel);
  }
  free(samples);

  free(gridData);
  free(sampleDensity);

  free(gridDataParallel);
  free(sampleDensityParallel);

  printf("\n");

  return 0;
}
