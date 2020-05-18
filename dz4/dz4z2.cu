#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ACCURACY 0.01

#define MAXLINE 128
#define PIXPERLINE 16

char c[MAXLINE];

void pgmsize(char* filename, int* nx, int* ny);
void pgmread(char* filename, void* vp, int nxmax, int nymax, int* nx, int* ny);
void pgmwrite(char* filename, void* vx, int nx, int ny);

double** dosharpen(char* infile, int nx, int ny);
//double** dosharpenParallel(char* infile, int nx, int ny);
double filter(int d, int i, int j);

int** int2Dmalloc(int nx, int ny);
double** double2Dmalloc(int nx, int ny);

//void compareSharp(int w, int h, double** sequential, double** parallel);

double wtime();
void pgmsize(char* filename, int* nx, int* ny)
{
	FILE* fp;

	if (NULL == (fp = fopen(filename, "r")))
	{
		fprintf(stderr, "pgmsize: cannot open <%s>\n", filename);
		exit(-1);
	}

	fgets(c, MAXLINE, fp);
	fgets(c, MAXLINE, fp);

	fscanf(fp, "%d %d", nx, ny);

	fclose(fp);
}

void pgmread(char* filename, void* vp, int nxmax, int nymax, int* nx, int* ny)
{
	FILE* fp;

	int nxt, nyt, i, j, t;

	int* pixmap = (int*)vp;

	if (NULL == (fp = fopen(filename, "r")))
	{
		fprintf(stderr, "pgmread: cannot open <%s>\n", filename);
		exit(-1);
	}

	fgets(c, MAXLINE, fp);
	fgets(c, MAXLINE, fp);

	fscanf(fp, "%d %d", nx, ny);

	nxt = *nx;
	nyt = *ny;

	if (nxt > nxmax || nyt > nymax)
	{
		fprintf(stderr, "pgmread: image larger than array\n");
		fprintf(stderr, "nxmax, nymax, nxt, nyt = %d, %d, %d, %d\n",
			nxmax, nymax, nxt, nyt);
		exit(-1);
	}

	fscanf(fp, "%d", &t);

	for (j = 0; j < nyt; j++)
	{
		for (i = 0; i < nxt; i++)
		{
			fscanf(fp, "%d", &t);
			pixmap[(nyt - j - 1) + nyt * i] = t;
		}
	}

	fclose(fp);
}

void pgmwrite(char* filename, void* vx, int nx, int ny)
{
	FILE* fp;

	int i, j, k, grey;

	double xmin, xmax, tmp;
	double thresh = 255.0;

	double* x = (double*)vx;

	if (NULL == (fp = fopen(filename, "w")))
	{
		fprintf(stderr, "pgmwrite: cannot create <%s>\n", filename);
		exit(-1);
	}

	xmin = fabs(x[0]);
	xmax = fabs(x[0]);

	for (i = 0; i < nx * ny; i++)
	{
		if (fabs(x[i]) < xmin)
			xmin = fabs(x[i]);
		if (fabs(x[i]) > xmax)
			xmax = fabs(x[i]);
	}

	fprintf(fp, "P2\n");
	fprintf(fp, "# Written by pgmwrite\n");
	fprintf(fp, "%d %d\n", nx, ny);
	fprintf(fp, "%d\n", (int)thresh);

	k = 0;

	for (j = ny - 1; j >= 0; j--)
	{
		for (i = 0; i < nx; i++)
		{

			tmp = x[j + ny * i];

			if (xmin < 0 || xmax > thresh)
			{
				tmp = (int)((thresh * ((fabs(tmp - xmin)) / (xmax - xmin))) + 0.5);
			}
			else
			{
				tmp = (int)(fabs(tmp) + 0.5);
			}

			grey = tmp;

			fprintf(fp, "%3d ", grey);

			if (0 == (k + 1) % PIXPERLINE)
				fprintf(fp, "\n");

			k++;
		}
	}

	if (0 != k % PIXPERLINE)
		fprintf(fp, "\n");
	fclose(fp);
}

double wtime(void)
{
	return 0;
}

double filter(int d, int i, int j)
{
	double rd4sq, rsq, sigmad4sq, sigmasq, x, y, delta;

	int d4 = 4;

	double sigmad4 = 1.4;
	double filter0 = -40.0;

	rd4sq = d4 * d4;
	rsq = d * d;

	sigmad4sq = sigmad4 * sigmad4;
	sigmasq = sigmad4sq * (rsq / rd4sq);

	x = (double)i;
	y = (double)j;

	rsq = x * x + y * y;

	delta = rsq / (2.0 * sigmasq);

	return (filter0 * (1.0 - delta) * exp(-delta));
}

int** int2Dmalloc(int nx, int ny)
{
	int i;
	int** idata;

	idata = (int**)malloc(nx * sizeof(int*) + nx * ny * sizeof(int));

	idata[0] = (int*)(idata + nx);

	for (i = 1; i < nx; i++)
	{
		idata[i] = idata[i - 1] + ny;
	}

	return idata;
}

double** double2Dmalloc(int nx, int ny)
{
	int i;
	double** ddata;

	ddata = (double**)malloc(nx * sizeof(double*) + nx * ny * sizeof(double));

	ddata[0] = (double*)(ddata + nx);

	for (i = 1; i < nx; i++)
	{
		ddata[i] = ddata[i - 1] + ny;
	}

	return ddata;
}

double** dosharpen(char* infile, int nx, int ny)
{
	int d = 8;

	double norm = (2 * d - 1) * (2 * d - 1);
	double scale = 2.0;

	int xpix, ypix, pixcount;

	int i, j, k, l;
	double tstart, tstop, time;

	int** fuzzy = int2Dmalloc(nx, ny);								/* Will store the fuzzy input image when it is first read in from file */
	double** fuzzyPadded = double2Dmalloc(nx + 2 * d, ny + 2 * d);  /* Will store the fuzzy input image plus additional border padding */
	double** convolutionPartial = double2Dmalloc(nx, ny);			/* Will store the convolution of the filter with parts of the fuzzy image computed by individual processes */
	double** convolution = double2Dmalloc(nx, ny);					/* Will store the convolution of the filter with the full fuzzy image */
	double** sharp = double2Dmalloc(nx, ny);						/* Will store the sharpened image obtained by adding rescaled convolution to the fuzzy image */
	double** sharpCropped = double2Dmalloc(nx - 2 * d, ny - 2 * d); /* Will store the sharpened image cropped to remove a border layer distorted by the algorithm */

	char outfile[256];
	strcpy(outfile, infile);
	*(strchr(outfile, '.')) = '\0';
	strcat(outfile, "_sharpened.pgm");

	for (i = 0; i < nx; i++)
	{
		for (j = 0; j < ny; j++)
		{
			fuzzy[i][j] = 0;
			sharp[i][j] = 0.0;
		}
	}

	// printf("Using a filter of size %d x %d\n", 2 * d + 1, 2 * d + 1);
	// printf("\n");

	// printf("Reading image file: %s\n", infile);
	// fflush(stdout);

	pgmread(infile, &fuzzy[0][0], nx, ny, &xpix, &ypix);

	// printf("... done\n\n");
	// fflush(stdout);

	if (xpix == 0 || ypix == 0 || nx != xpix || ny != ypix)
	{
		printf("Error reading %s\n", infile);
		fflush(stdout);
		exit(-1);
	}

	for (i = 0; i < nx + 2 * d; i++)
	{
		for (j = 0; j < ny + 2 * d; j++)
		{
			fuzzyPadded[i][j] = 0.0;
		}
	}

	for (i = 0; i < nx; i++)
	{
		for (j = 0; j < ny; j++)
		{
			fuzzyPadded[i + d][j + d] = fuzzy[i][j];
		}
	}

	// printf("Starting calculation ...\n");

	// fflush(stdout);

	tstart = wtime();

	pixcount = 0;

	for (i = 0; i < nx; i++)
	{
		for (j = 0; j < ny; j++)
		{
			for (k = -d; k <= d; k++)
			{
				for (l = -d; l <= d; l++)
				{
					convolution[i][j] = convolution[i][j] + filter(d, k, l) * fuzzyPadded[i + d + k][j + d + l];
				}
			}
			pixcount += 1;
		}
	}

	tstop = wtime();
	time = tstop - tstart;

	// printf("... finished\n");
	// printf("\n");
	// fflush(stdout);

	for (i = 0; i < nx; i++)
	{
		for (j = 0; j < ny; j++)
		{
			sharp[i][j] = fuzzyPadded[i + d][j + d] - scale / norm * convolution[i][j];
		}
	}

	// printf("Writing output file: %s\n", outfile);
	// printf("\n");

	for (i = d; i < nx - d; i++)
	{
		for (j = d; j < ny - d; j++)
		{
			sharpCropped[i - d][j - d] = sharp[i][j];
		}
	}

	pgmwrite(outfile, &sharpCropped[0][0], nx - 2 * d, ny - 2 * d);

	// printf("... done\n");
	// printf("\n");
	// printf("Calculation time was %f seconds\n", time);
	// fflush(stdout);

	free(fuzzy);
	free(fuzzyPadded);
	free(convolutionPartial);
	free(convolution);
	// free(sharp);
	free(sharpCropped);

	return sharp;
}

double** makeFilterMatrix(int d)
{
    double** matrix = (double**)malloc((2 * d + 1) * sizeof(double*));

    for (int i = 0; i <= 2 * d; i++)
    {
        matrix[i] = (double*)malloc((2 * d + 1) * sizeof(double));

        for (int j = -d; j <= d; j++)
            matrix[i][j + d] = filter(d, i - d, j);
    }

    return matrix;
}

#define TILE_SIZE 16

__global__
void sharpenKernel(double* filterMatrix, double* fuzzyPadded, double* convolution, int nx, int ny)
{
    // __shared__ double sharedFuzzyPadded[32][32];
    /*int misses = 0;
    for (int k = -d; k <= d; k++)
        for (int l = -d; l <= d; l++) {
            double t1 = dev_FilterMatrix[(k + d) * 17 + l + d];
            double t2 = filter(8, k, l);

            if (t1 != t2)
                misses++;

        }*/

    const int d = 8;
    const double norm = (2 * d - 1) * (2 * d - 1);
    int i = blockIdx.y * TILE_SIZE + threadIdx.y;
    int j = blockIdx.x * TILE_SIZE + threadIdx.x;

    /*if (threadIdx.x == 0 && threadIdx.y == 0)
    {
        for (int k = -d; k <= d; k++)
            for (int l = -d; l <= d; l++)
                sharedFuzzyPadded[i + d + k][j + d + l] = fuzzyPadded[i + d + k][j + d + l];
    }
    __syncthreads();*/

    if (i < nx && j < ny) {
        //sharp[i][j] = 0.0;

        for (int k = -d; k <= d; k++)
        {
            for (int l = -d; l <= d; l++)
            {
                convolution[i * ny + j] = convolution[i * ny + j] + filterMatrix[(k + d) * (17) + l + d] * fuzzyPadded[(i + d + k) * (ny + 2 * d) + j + d + l];
            }
        }

        convolution[i * ny + j] *= (2.0 / norm);
    }
}

double** sharpen_cuda_init(char* infile, int nx, int ny, float* ms)
{
    const int d = 8;
    //////////////////////////////////////
    // LUT

    double* dev_FilterMatrix;



    double** filterMatrix = makeFilterMatrix(d);
    size_t filterMatrixSize = (2 * d + 1) * (2 * d + 1) * sizeof(double);
    cudaMalloc(&dev_FilterMatrix, filterMatrixSize);
    cudaMemcpy(dev_FilterMatrix, &filterMatrix[0][0], filterMatrixSize, cudaMemcpyHostToDevice);
    //cudaMemcpyToSymbol(dev_FilterMatrix, &filterMatrix[0][0], filterMatrixSize);
    //////////////////////////////////////

    double** sharp = double2Dmalloc(nx, ny);
    double** sharpCropped = double2Dmalloc(nx - 2 * d, ny - 2 * d);
    int** fuzzy = int2Dmalloc(nx, ny);
    double** fuzzyPadded = double2Dmalloc(nx + 2 * d, ny + 2 * d);
    pgmread(infile, &fuzzy[0][0], nx, ny, &nx, &ny);

    for (int i = 0; i < nx; i++)
    {
        for (int j = 0; j < ny; j++)
        {
            fuzzy[i][j] = 0;
            sharp[i][j] = 0.0;
        }
    }

    for (int i = 0; i < nx + 2 * d; i++)
    {
        for (int j = 0; j < ny + 2 * d; j++)
        {
            fuzzyPadded[i][j] = 0.0;
        }
    }

    for (int i = 0; i < nx; i++)
        for (int j = 0; j < ny; j++)
            fuzzyPadded[i + d][j + d] = fuzzy[i][j];

    double* devFuzzyPadded;
    size_t devFuzzyPadded_size = (nx + 2 * d) * (ny + 2 * d) * sizeof(double);

    cudaMalloc(&devFuzzyPadded, devFuzzyPadded_size);
    cudaMemcpy(devFuzzyPadded, &fuzzyPadded[0][0], devFuzzyPadded_size, cudaMemcpyHostToDevice);

    //////////////////////////////////////

    //double** devSharpCropped;
    //size_t sharpCroppedSize = (nx - 2 * d) * (ny - 2 * d) * sizeof(double);
    //cudaMalloc(&devSharpCropped, sharpCroppedSize);
    //////////////////////////////////////

    double** convolution = double2Dmalloc(nx, ny);

    size_t convolutionSize = (nx) * (ny) * sizeof(double);
    size_t sharpSize = (nx) * (ny) * sizeof(double);
    double* devConvolution;
    cudaMalloc(&devConvolution, convolutionSize);

    int gridX_size = ceil(nx / TILE_SIZE);
    int gridY_size = ceil(ny / TILE_SIZE);

    dim3 dimGrid(gridX_size, gridY_size);
    dim3 dimBlock(TILE_SIZE, TILE_SIZE);

    cudaEvent_t start, stop;
    cudaEventCreate(&start);
    cudaEventCreate(&stop);

    cudaEventRecord(start);
    sharpenKernel << <dimGrid, dimBlock >> > (dev_FilterMatrix, devFuzzyPadded, devConvolution, nx, ny);
    cudaEventRecord(stop);

    cudaEventSynchronize(stop);
    cudaEventElapsedTime(ms, start, stop);


    cudaMemcpy(&convolution[0][0], devConvolution, convolutionSize, cudaMemcpyDeviceToHost);


    for (int i = 0; i < nx; i++)
        for (int j = 0; j < ny; j++)
            sharp[i][j] = fuzzyPadded[i + d][j + d] - convolution[i][j];

    for (int i = d; i < nx - d; i++)
        for (int j = d; j < ny - d; j++)
            sharpCropped[i - d][j - d] = sharp[i][j];

    cudaFree(dev_FilterMatrix);
    cudaFree(devFuzzyPadded);
    //cudaFree(devSharpCropped);
    cudaFree(devConvolution);
    //cudaFree(devSharp);

    free(fuzzy);
    free(fuzzyPadded);
    free(convolution);
    free(sharp);

    return sharpCropped;
}

void compareSharp(int w, int h, double** sequential, double** parallel)
{
    int misses = 0;

    for (int i = 0; i < w; i++)
    {
        for (int j = 0; j < h; j++)
        {
            if (fabs(sequential[i][j] - parallel[i][j]) > ACCURACY)
                misses++;
        }
    }

    if (misses > ACCURACY * (w * h))
        printf("Test FAILED\n");
    else
        printf("Test PASSED\n");
}

int main(int argc, char* argv[])
{
	double tstart, tstop, time, timeParallel;

	char* filename;
	int xpix, ypix;

	if (argc < 2)
		return 1;

	filename = argv[1];

	// printf("\n");
	// printf("Image sharpening code running in serial\n");
	// printf("\n");
	// printf("Input file is: %s\n", filename);

	pgmsize(filename, &xpix, &ypix);

	// printf("Image size is %d x %d\n", xpix, ypix);
	// printf("\n");

	tstart = wtime();

	double** sharpSequential = dosharpen(filename, xpix, ypix);

	tstop = wtime();
	time = tstop - tstart;

  tstart = wtime();
  
  float ms;
	double** sharpParallel = sharpen_cuda_init(filename, xpix, ypix, &ms);

	tstop = wtime();
	timeParallel = tstop - tstart;

	printf("Input file: %s\n", filename);
	printf("Sequential execution time: %f\n", time);
	printf("Parallel execution time: %f\n", timeParallel);

	compareSharp(xpix, ypix, sharpSequential, sharpParallel);

	printf("\n");

	free(sharpSequential);
	//free(sharpParallel);

	return 0;
}
