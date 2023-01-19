CC=gcc

CFLAGS=-g3

OFLAGS=-march=native -O1

OFLAGS2=-march=native -O3 -ffast-math -funroll-loops -fopenmp

IFLAGS=-I./include/

DIRSRC=./src

all: sobel_baseline sobel_compil_flags sobel_grey sobel_sqrtless

sobel_baseline: $(DIRSRC)/sobel_baseline.c $(DIRSRC)/common.c
	$(CC) $(CFLAGS) $(OFLAGS) $(IFLAGS) $^ -o $@ -lm

sobel_compil_flags: $(DIRSRC)/sobel_baseline.c $(DIRSRC)/common.c
	$(CC) $(CFLAGS) $(OFLAGS2) $(IFLAGS) $^ -o $@ -lm

sobel_grey: $(DIRSRC)/sobel_grey.c $(DIRSRC)/common.c
	$(CC) $(CFLAGS) $(OFLAGS2) $(IFLAGS) $^ -o $@ -lm

sobel_sqrtless: $(DIRSRC)/sobel_sqrtless.c $(DIRSRC)/common.c
	$(CC) $(CFLAGS) $(OFLAGS2) $(IFLAGS) $^ -o $@ -lm

clean:
	rm -Rf *~ sobel_* 
