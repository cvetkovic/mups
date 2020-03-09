# C compiler
CC = gcc
CC_FLAGS = -fopenmp -O2

mri-gridding: 
	$(CC) $(CC_FLAGS) main.c CPU_kernels.c -o mri-gridding -lm 

clean:
	rm -f mri-gridding
