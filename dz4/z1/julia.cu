# include <stdio.h>
# include <stdlib.h>
# include <string.h>
# include <time.h>

#include "cuda_runtime.h"
#include "device_launch_parameters.h"

# define DEFAULT_H 1000
# define DEFAULT_W 1000
# define DEFAULT_CNT 200
# define DEFAULT_FILENAME "julia"

int main(int argc, char* argv[]);
unsigned char* julia_set(int w, int h, int cnt, float xl, float xr, float yb, float yt);
unsigned char* julia_set_parallel(int w, int h, int cnt, float xl, float xr, float yb, float yt);
int julia(int w, int h, float xl, float xr, float yb, float yt, int i, int j, int cnt);
void julia_parallel(int w, int h, float xl, float xr, float yb, float yt, int i, int j, int cnt, int* val);
void tga_write(int w, int h, unsigned char rgb[], char* filename);
void timestamp();

#define ACCURACY 0.01

void tga_compare(int w, int h, unsigned char* rgb_sequential, unsigned char* rgb_parallel)
{
	char failed = 0;

	for (int i = 0; i < 3 * w * h; i++)
	{
		if (abs(rgb_sequential[i] - rgb_parallel[i]) > ACCURACY)
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

int main(int argc, char* argv[]) {
	int h = DEFAULT_H;
	int w = DEFAULT_W;
	int cnt = DEFAULT_CNT;
	char filename[256] = DEFAULT_FILENAME;
	char buffer[256];
	unsigned char* rgb;
	unsigned char* rgbParallel;
	double t1;
	double t2;
	float xl = -1.5;
	float xr = +1.5;
	float yb = -1.5;
	float yt = +1.5;

	if (argc == 4) {
		h = atoi(argv[1]);
		w = atoi(argv[2]);
		cnt = atoi(argv[3]);
		if (!h || !w || !cnt) return 1;
	}

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

	timestamp();
	printf("--------------------------------------------------------------\n");
	printf("JULIA Set\n");
	printf("  Plot a version of the Julia set for Z(k+1)=Z(k)^2-0.8+0.156i\n");

	//double timeSequential = timestamp();
	rgb = julia_set(w, h, cnt, xl, xr, yb, yt);
	//timeSequential = timestamp() - timeSequential;

	//double timeParallel = timestamp();
	rgbParallel = julia_set_parallel(w, h, cnt, xl, xr, yb, yt);
	//timeParallel = timestamp() - timeParallel;

	//printf("\Sequential execution time: %f\n", timeSequential);
	//printf("\tParallel execution time: %f\n", timeParallel);

	tga_compare(w, h, rgb, rgbParallel);
	tga_write(w, h, rgbParallel, filename);

	free(rgb);
	free(rgbParallel);

	printf("\n");
	printf("JULIA set:\n");
	printf("  Normal end of execution.\n");

	timestamp();

	return 0;
}

unsigned char* julia_set(int w, int h, int cnt, float xl, float xr, float yb, float yt)
{
	int i;
	int j;
	int juliaValue;
	int k;
	unsigned char* rgb;

	rgb = (unsigned char*)malloc(w * h * 3 * sizeof(unsigned char));

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

int julia(int w, int h, float xl, float xr, float yb, float yt, int i, int j, int cnt)
{
	float ai;
	float ar;
	float ci = 0.156;
	float cr = -0.8;
	int k;
	float t;
	float x;
	float y;

	x = ((float)(w - i - 1) * xl
		+ (float)(i)*xr)
		/ (float)(w - 1);

	y = ((float)(h - j - 1) * yb
		+ (float)(j)*yt)
		/ (float)(h - 1);

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

__global__
void juliaValueKernel(void* rgb_void, int w, int h, int cnt, float xl, float xr, float yb, float yt)
{
	unsigned char* rgb = (unsigned char*)rgb_void;
	int i, j;

	j = threadIdx.x;
	i = blockIdx.x;

	/*for (int k = 0; k < w * h; k++)
		rgb[k] = 128;*/

		/*for (j = 0; j < w; j++) {
			for (i = 0; i < h; i++) {*/
	int juliaValue;
	julia_parallel(w, h, xl, xr, yb, yt, i, j, cnt, &juliaValue);

	int k = 3 * (j * w + i);

	rgb[k] = 255 * (1 - juliaValue);
	rgb[k + 1] = 255 * (1 - juliaValue);
	rgb[k + 2] = 255;
	/*}
}*/
}

unsigned char* julia_set_parallel(int w, int h, int cnt, float xl, float xr, float yb, float yt)
{
	int i;
	int j;
	int juliaValue;
	int k;

	unsigned char* rgb;
	void* dev_rgb;

	size_t size_rgb = w * h * 3 * sizeof(unsigned char);

	rgb = (unsigned char*)malloc(size_rgb);
	cudaMalloc(&dev_rgb, size_rgb);

	// make two h times w blocks

	//juliaValueKernel((void*)rgb, w, h, cnt, xl, xr, yb, yt);
	juliaValueKernel << < h, w >> > ((void*)dev_rgb, w, h, cnt, xl, xr, yb, yt);
	//cudaDeviceSynchronize();

	cudaMemcpy(rgb, dev_rgb, size_rgb, cudaMemcpyDeviceToHost);
	cudaFree(dev_rgb);

	return rgb;
}

__device__
void julia_parallel(int w, int h, float xl, float xr, float yb, float yt, int i, int j, int cnt, int* val)
{
	float ai;
	float ar;
	float ci = 0.156;
	float cr = -0.8;
	int k;
	float t;
	float x;
	float y;

	x = ((float)(w - i - 1) * xl
		+ (float)(i)*xr)
		/ (float)(w - 1);

	y = ((float)(h - j - 1) * yb
		+ (float)(j)*yt)
		/ (float)(h - 1);

	ar = x;
	ai = y;

	int returnValue = 1;

	for (k = 0; k < cnt; k++)
	{
		t = ar * ar - ai * ai + cr;
		ai = ar * ai + ai * ar + ci;
		ar = t;

		if (1000 < ar * ar + ai * ai)
		{
			returnValue = 0;
			break;
		}
	}

	*val = returnValue;
}

void tga_write(int w, int h, unsigned char rgb[], char* filename)
{
	FILE* file_unit;
	unsigned char header1[12] = { 0,0,2,0,0,0,0,0,0,0,0,0 };
	unsigned char header2[6] = { w % 256, w / 256, h % 256, h / 256, 24, 0 };

	file_unit = fopen(filename, "wb");

	fwrite(header1, sizeof(unsigned char), 12, file_unit);
	fwrite(header2, sizeof(unsigned char), 6, file_unit);

	fwrite(rgb, sizeof(unsigned char), 3 * w * h, file_unit);

	fclose(file_unit);

	printf("\n");
	printf("TGA_WRITE:\n");
	printf("  Graphics data saved as '%s'\n", filename);

	return;
}

void timestamp(void)
{
# define TIME_SIZE 40

	static char time_buffer[TIME_SIZE];
	const struct tm* tm;
	time_t now;

	now = time(NULL);
	tm = localtime(&now);

	strftime(time_buffer, TIME_SIZE, "%d %B %Y %I:%M:%S %p", tm);

	printf("%s\n", time_buffer);

	return;
# undef TIME_SIZE
}
