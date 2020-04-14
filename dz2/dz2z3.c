#include "dz2z3.h"

void pgmsize(char *filename, int *nx, int *ny)
{
	FILE *fp;

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

void pgmread(char *filename, void *vp, int nxmax, int nymax, int *nx, int *ny)
{
	FILE *fp;

	int nxt, nyt, i, j, t;

	int *pixmap = (int *)vp;

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

void pgmwrite(char *filename, void *vx, int nx, int ny)
{
	FILE *fp;

	int i, j, k, grey;

	double xmin, xmax, tmp;
	double thresh = 255.0;

	double *x = (double *)vx;

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
	struct timeval tp;
	gettimeofday(&tp, NULL);
	return tp.tv_sec + tp.tv_usec / (double)1.0e6;
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

int **int2Dmalloc(int nx, int ny)
{
	int i;
	int **idata;

	idata = (int **)malloc(nx * sizeof(int *) + nx * ny * sizeof(int));

	idata[0] = (int *)(idata + nx);

	for (i = 1; i < nx; i++)
	{
		idata[i] = idata[i - 1] + ny;
	}

	return idata;
}

double **double2Dmalloc(int nx, int ny)
{
	int i;
	double **ddata;

	ddata = (double **)malloc(nx * sizeof(double *) + nx * ny * sizeof(double));

	ddata[0] = (double *)(ddata + nx);

	for (i = 1; i < nx; i++)
	{
		ddata[i] = ddata[i - 1] + ny;
	}

	return ddata;
}

double **dosharpen(char *infile, int nx, int ny)
{
	int d = 8;

	double norm = (2 * d - 1) * (2 * d - 1);
	double scale = 2.0;

	int xpix, ypix, pixcount;

	int i, j, k, l;
	double tstart, tstop, time;

	int **fuzzy = int2Dmalloc(nx, ny);								/* Will store the fuzzy input image when it is first read in from file */
	double **fuzzyPadded = double2Dmalloc(nx + 2 * d, ny + 2 * d);	/* Will store the fuzzy input image plus additional border padding */
	double **convolutionPartial = double2Dmalloc(nx, ny);			/* Will store the convolution of the filter with parts of the fuzzy image computed by individual processes */
	double **convolution = double2Dmalloc(nx, ny);					/* Will store the convolution of the filter with the full fuzzy image */
	double **sharp = double2Dmalloc(nx, ny);						/* Will store the sharpened image obtained by adding rescaled convolution to the fuzzy image */
	double **sharpCropped = double2Dmalloc(nx - 2 * d, ny - 2 * d); /* Will store the sharpened image cropped to remove a border layer distorted by the algorithm */

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

double **makeFilterMatrix(int d)
{
	double **matrix = (double **)malloc((2 * d + 1) * sizeof(double *));

	for (int i = 0; i <= 2 * d; i++)
	{
		matrix[i] = (double *)malloc((2 * d + 1) * sizeof(double));

		for (int j = -d; j <= d; j++)
			matrix[i][j + d] = filter(d, i - d, j);
	}

	return matrix;
}

void dosharpenParallel(int nx, int ny, int d, int start, int end, double **convolution, double **fuzzyPadded, double **sharp, double **sharpCropped)
{
	double norm = (2 * d - 1) * (2 * d - 1);
	double scale = 2.0;

	double **filterMatrix = makeFilterMatrix(d);

	for (int count = start; count < end; count++)
	{
		int i = count / ny;
		int j = count % ny;

		for (int k = -d; k <= d; k++)
		{
			for (int l = -d; l <= d; l++)
			{
				convolution[i][j] = convolution[i][j] + filterMatrix[k + d][l + d] * fuzzyPadded[i + d + k][j + d + l];
			}
		}

		double c = scale / norm;
		sharp[i][j] = fuzzyPadded[i + d][j + d] - c * convolution[i][j];

		if (i < nx - 2 * d && j < ny - 2 * d)
			sharpCropped[i][j] = sharp[i + d][j + d];
	}

	free(filterMatrix);
}

void compareSharp(int w, int h, double **sequential, double **parallel)
{
	char failed = 0;

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

int main(int argc, char *argv[])
{
	const int d = 8;

	int rank, size;
	double tstart, tstop, time, timeParallel;

	int chunk, start, end;

	char *filename;
	int nx, ny;

	char outfile[256];

	int **fuzzy;
	double **fuzzyPadded;
	double **sharp;
	double **sharpReduced;
	double **sharpParallel;
	double **sharpCropped;
	double **convolution;

	MPI_Init(&argc, &argv);

	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &size);

	if (rank == MASTER_RANK)
	{
		if (size != N)
		{
			fprintf(stderr, "Invalid number of processes!\n");
			fprintf(stderr, "Active count: %d | Target count: %d\n", size, N);
			MPI_Abort(MPI_COMM_WORLD, -1);
		}

		if (argc < 2)
			MPI_Abort(MPI_COMM_WORLD, 1);

		filename = argv[1];
		pgmsize(filename, &nx, &ny);

		fuzzy = int2Dmalloc(nx, ny);
		sharpReduced = double2Dmalloc(nx, ny);
		sharpParallel = double2Dmalloc(nx - 2 * d, ny - 2 * d);

		tstart = wtime();
	}

	MPI_Bcast(&nx, 1, MPI_INT, MASTER_RANK, MPI_COMM_WORLD);
	MPI_Bcast(&ny, 1, MPI_INT, MASTER_RANK, MPI_COMM_WORLD);

	fuzzyPadded = double2Dmalloc(nx + 2 * d, ny + 2 * d);
	sharp = double2Dmalloc(nx, ny);
	sharpCropped = double2Dmalloc(nx - 2 * d, ny - 2 * d);
	convolution = double2Dmalloc(nx, ny);

	for (int i = 0; i < nx; i++)
	{
		for (int j = 0; j < ny; j++)
		{
			if (rank == MASTER_RANK) fuzzy[i][j] = 0;
			sharp[i][j] = 0.0;
		}
	}

	if (rank == MASTER_RANK)
	{
		strcpy(outfile, filename);
		*(strchr(outfile, '.')) = '\0';
		strcat(outfile, "_mpi_sharpened.pgm");

		int xpix, ypix;

		pgmread(filename, &fuzzy[0][0], nx, ny, &xpix, &ypix);

		if (xpix == 0 || ypix == 0 || nx != xpix || ny != ypix)
		{
			printf("Error reading %s\n", filename);
			fflush(stdout);
			exit(-1);
		}

		for (int i = 0; i < nx + 2 * d; i++)
		{
			for (int j = 0; j < ny + 2 * d; j++)
			{
				fuzzyPadded[i][j] = 0.0;
			}
		}

		for (int i = 0; i < nx; i++)
		{
			for (int j = 0; j < ny; j++)
			{
				fuzzyPadded[i + d][j + d] = fuzzy[i][j];
			}
		}
	}

	MPI_Bcast(&fuzzyPadded[0][0], (nx + 2 * d) * (ny + 2 * d), MPI_DOUBLE, MASTER_RANK, MPI_COMM_WORLD);

	chunk = (nx * ny + size - 1) / size;
	start = rank * chunk;
	end = start + chunk < nx * ny ? start + chunk : nx * ny;

	dosharpenParallel(nx, ny, d, start, end, convolution, fuzzyPadded, sharp, sharpCropped);

	MPI_Reduce(sharp, sharpReduced, nx * ny, MPI_DOUBLE, MPI_SUM, MASTER_RANK, MPI_COMM_WORLD);
	MPI_Reduce(sharpCropped, sharpParallel, (nx - 2 * d) * (ny - 2 * d), MPI_DOUBLE, MPI_SUM, MASTER_RANK, MPI_COMM_WORLD);

	if (rank == MASTER_RANK)
	{
		tstop = wtime();
		timeParallel = tstop - tstart;

		pgmwrite(outfile, &sharpParallel[0][0], nx - 2 * d, ny - 2 * d);

		tstart = wtime();

		double **sharpSequential = dosharpen(filename, nx, ny);

		tstop = wtime();
		time = tstop - tstart;

		printf("Input file: %s\n", filename);
		printf("Number of threads: %d\n", size);
		printf("Sequential execution time: %f\n", time);
		printf("Parallel execution time: %f\n", timeParallel);

		compareSharp(nx, ny, sharpSequential, sharpReduced);

		putchar('\n');

		free(fuzzy);
		free(sharpReduced);
		free(sharpParallel);
		free(sharpSequential);
	}

	free(fuzzyPadded);
	free(convolution);
	free(sharp);
	free(sharpCropped);

	MPI_Finalize();

	return 0;
}