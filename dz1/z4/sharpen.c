#include <stdio.h>
#include <stdlib.h>
#include "sharpen.h"
#include "utilities.h"

int main(int argc, char *argv[])
{
  double tstart, tstop, time, timeParallel;
  
  char *filename;
  int xpix, ypix;
  
  if (argc < 2) return 1;
  
  filename = argv[1];
    
  /*printf("\n");
  printf("Image sharpening code running in serial\n");
  printf("\n");
  printf("Input file is: %s\n", filename);*/
  
  pgmsize(filename, &xpix, &ypix);
  
  /*printf("Image size is %d x %d\n", xpix, ypix);
  printf("\n");*/
  
  tstart  = wtime();
  
  double** sharpSequential = dosharpen(filename, xpix, ypix);
  
  tstop = wtime();
  time  = tstop - tstart;
  
  // parallel
  //printf("Image sharpening code running in serial\n");
  tstart  = wtime();
  
  double** sharpParallel = dosharpenParallel(filename, xpix, ypix);
  
  tstop = wtime();
  timeParallel  = tstop - tstart;

  printf("Sequential time: %f\n", time);
  printf("Parallel time: %f\n", timeParallel);
  
  compareSharp(xpix, ypix, sharpSequential, sharpParallel);
  
  free(sharpSequential);
  free(sharpParallel);

  return 0;
}

void compareSharp(int w, int h, double** sequential, double** parallel)
{
  int failed = 0;

  for (int i = 0; i < w; i++)
  {
    for (int j = 0; j < h; j++)
    {
      if (fabs(sequential[i][j] - parallel[i][j]) > ACCURACY)
      {
        failed = 1;
        break;
      }
    }
  }

	if (failed)
		printf("Test FAILED\n");
	else
		printf("Test PASSED\n");
}