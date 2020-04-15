#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define DEFAULT_H 1000
#define DEFAULT_W 1000
#define DEFAULT_CNT 200
#define DEFAULT_FILENAME "dz2z2"

#define N 4
#define ACCURACY 0.01

#define MASTER_RANK 0

enum Tags
{
	Y_COORDINATE_TAG,
	RESULT_TAG,
	STOP_TAG
};

int main(int argc, char *argv[]);
unsigned char *julia_set(int w, int h, int cnt, float xl, float xr, float yb, float yt);
unsigned char *julia_set_parallel(int w, int h, int j, int cnt, float xl, float xr, float yb, float yt);
int julia(int w, int h, float xl, float xr, float yb, float yt, int i, int j, int cnt);
void tga_write(int w, int h, unsigned char rgb[], char *filename);
void tga_compare(int w, int h, unsigned char *rgbSequential, unsigned char *rgbParallel);
void timestamp();

int main(int argc, char *argv[])
{
	int h, w, cnt;
	int rank, size;
	
	unsigned char *rgbParallel;

    double timeSequential;
    double timeParallel;

	const float xl = -1.5;
	const float xr = +1.5;
	const float yb = -1.5;
	const float yt = +1.5;

	MPI_Datatype resultType;

	MPI_Init(&argc, &argv);

	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &size);

	if (rank == MASTER_RANK)
	{
		if (size != N)
		{
			fprintf(stderr, "Invalid number of processes!\n");
			fprintf(stderr, "Active count: %d | Target count: %d\n", size, N);
			MPI_Abort(MPI_COMM_WORLD, 127);
			return 1;
		}

		h = DEFAULT_H;
		w = DEFAULT_W;
		cnt = DEFAULT_CNT;

		if (argc == 4)
		{
			h = atoi(argv[1]);
			w = atoi(argv[2]);
			cnt = atoi(argv[3]);
			
			if (!h || !w || !cnt)
			{
				MPI_Abort(MPI_COMM_WORLD, 130);
				return 1;
			}
		}

		rgbParallel = (unsigned char *)malloc(3 * w * h * sizeof(unsigned char));

		timeParallel = MPI_Wtime();
	}

	MPI_Bcast(&h, 1, MPI_INT, MASTER_RANK, MPI_COMM_WORLD);
	MPI_Bcast(&w, 1, MPI_INT, MASTER_RANK, MPI_COMM_WORLD);
	MPI_Bcast(&cnt, 1, MPI_INT, MASTER_RANK, MPI_COMM_WORLD);

	MPI_Type_contiguous(3 * w, MPI_UNSIGNED_CHAR, &resultType);
	MPI_Type_commit(&resultType);

	if (rank == MASTER_RANK)
	{
		int sentCount = 0;
		int processedCount = 0;

		while (sentCount < size - 1)
		{
			MPI_Send(&sentCount, 1, MPI_INT, sentCount + 1, Y_COORDINATE_TAG, MPI_COMM_WORLD);
			sentCount++;
		}

		while (processedCount < h)
		{
			int coordinate;
			MPI_Status status;

			MPI_Recv(&coordinate, 1, MPI_INT, MPI_ANY_SOURCE, Y_COORDINATE_TAG, MPI_COMM_WORLD, &status);

			int slaveRank = status.MPI_SOURCE;

			MPI_Recv(&rgbParallel[3 * w * coordinate], 1, resultType, slaveRank, RESULT_TAG, MPI_COMM_WORLD, &status);
			processedCount++;

			if (sentCount < h)
			{
				MPI_Send(&sentCount, 1, MPI_INT, slaveRank, Y_COORDINATE_TAG, MPI_COMM_WORLD);
				sentCount++;
			}
			else
			{
				MPI_Request request;
				MPI_Isend(&sentCount, 1, MPI_INT, slaveRank, STOP_TAG, MPI_COMM_WORLD, &request);
			}
		}

		timeParallel = MPI_Wtime() - timeParallel;

		char filename[256] = DEFAULT_FILENAME;
		char buffer[256];

		unsigned char *rgb;
		
		strcat(filename, "_");
		sprintf(buffer, "%d", h);
		strcat(filename, buffer);
		strcat(filename, "_");
		sprintf(buffer, "%d", w);
		strcat(filename, buffer);
		strcat(filename, "_");
		sprintf(buffer, "%d", cnt);
		strcat(filename, buffer);
		strcat(filename, ".tga");

		timeSequential = MPI_Wtime();
		rgb = julia_set(w, h, cnt, xl, xr, yb, yt);
		timeSequential = MPI_Wtime() - timeSequential;

		printf("Input values: %d %d %d\n", h, w, cnt);
		printf("Number of threads: %d\n", size);
		printf("Sequential execution time: %f\n", timeSequential);
		printf("Parallel execution time: %f\n", timeParallel);

		tga_write(w, h, rgbParallel, filename);

		tga_compare(w, h, rgb, rgbParallel);

		putchar('\n');

		free(rgb);
    	free(rgbParallel);
	}
	else // slave
	{
		int coordinate;

		while (1)
		{
			MPI_Status status;
			MPI_Recv(&coordinate, 1, MPI_INT, MASTER_RANK, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
			
			if (status.MPI_TAG == STOP_TAG)
			{
				break;
			}
			else
			{
				unsigned char *result = julia_set_parallel(w, h, coordinate, cnt, xl, xr, yb, yt);

				MPI_Request request;
				MPI_Isend(&coordinate, 1, MPI_INT, MASTER_RANK, Y_COORDINATE_TAG, MPI_COMM_WORLD, &request);
				MPI_Isend(result, 1, resultType, MASTER_RANK, RESULT_TAG, MPI_COMM_WORLD, &request);

				free(result);
			}
		}
	}

	MPI_Type_free(&resultType);
	MPI_Finalize();

	return 0;
}

unsigned char *julia_set(int w, int h, int cnt, float xl, float xr, float yb, float yt)
{
	int i;
	int j;
	int juliaValue;
	int k;
	unsigned char *rgb;

	rgb = (unsigned char *)malloc(3 * w * h * sizeof(unsigned char));

	for (j = 0; j < h; j++)
	{
		for (i = 0; i < w; i++)
		{
			juliaValue = julia(w, h, xl, xr, yb, yt, i, j, cnt);

			k = 3 * (j * w + i);

			rgb[k] = 255 * (1 - juliaValue);
			rgb[k + 1] = 255 * (1 - juliaValue);
			rgb[k + 2] = 255;
		}
	}

	return rgb;
}

unsigned char *julia_set_parallel(int w, int h, int j, int cnt, float xl, float xr, float yb, float yt)
{
	int i;
	int juliaValue;
	int k;
	unsigned char *rgb;

	rgb = (unsigned char *)malloc(3 * w * sizeof(unsigned char));

	for (i = 0; i < w; i++)
	{
		juliaValue = julia(w, h, xl, xr, yb, yt, i, j, cnt);

		k = 3 * i;

		rgb[k] = 255 * (1 - juliaValue);
		rgb[k + 1] = 255 * (1 - juliaValue);
		rgb[k + 2] = 255;
	}

	return rgb;
}

int julia(int w, int h, float xl, float xr, float yb, float yt, int i, int j, int cnt)
{
	float ai;
	float ar;
	const float ci = 0.156;
	const float cr = -0.8;
	int k;
	float t;
	float x;
	float y;

	x = ((float)(w - i - 1) * xl + (float)(i)*xr) / (float)(w - 1);

	y = ((float)(h - j - 1) * yb + (float)(j)*yt) / (float)(h - 1);

	ar = x;
	ai = y;

	for (k = 0; k < cnt; k++)
	{
		t = ar * ar - ai * ai + cr;
		ai = ar * ai + ai * ar + ci;
		ar = t;

		if (1000 < ar * ar + ai * ai)
		{
			return 0;
		}
	}

	return 1;
}

void tga_write(int w, int h, unsigned char rgb[], char *filename)
{
	FILE *file_unit;
	unsigned char header1[12] = {0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0};
	unsigned char header2[6] = {w % 256, w / 256, h % 256, h / 256, 24, 0};

	file_unit = fopen(filename, "wb");

	fwrite(header1, sizeof(unsigned char), 12, file_unit);
	fwrite(header2, sizeof(unsigned char), 6, file_unit);

	fwrite(rgb, sizeof(unsigned char), 3 * w * h, file_unit);

	fclose(file_unit);

	return;
}

void tga_compare(int w, int h, unsigned char *rgbSequential, unsigned char *rgbParallel)
{
    char failed = 0;

    for (int i = 0; i < 3 * w * h; i++)
    {
        if (abs(rgbSequential[i] - rgbParallel[i]) > ACCURACY)
        {
            failed = 1;
            break;
        }
    }

    if (failed)
        printf("Test FAILED\n");
    else
        printf("Test PASSED\n");
}

void timestamp(void)
{
#define TIME_SIZE 40

	static char time_buffer[TIME_SIZE];
	const struct tm *tm;
	time_t now;

	now = time(NULL);
	tm = localtime(&now);

	strftime(time_buffer, TIME_SIZE, "%d %B %Y %I:%M:%S %p", tm);

	printf("%s\n", time_buffer);

	return;
#undef TIME_SIZE
}