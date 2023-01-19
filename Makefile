CC=gcc

CFLAGS=-g3

OFLAGS=-march=native -O1

OFLAGS2=-march=native -O3 -ffast-math -funroll-loops -ftree-vectorize

IFLAGS=-I./include/

DIRSRC=./src

all: sobel_baseline sobel_compilflags sobel_grey sobel_sqrtless sobel_io sobel_parallel

sobel_baseline: $(DIRSRC)/sobel_baseline.c $(DIRSRC)/common.c
	$(CC) $(CFLAGS) $(OFLAGS) $(IFLAGS) $^ -o $@ -lm

sobel_compilflags: $(DIRSRC)/sobel_baseline.c $(DIRSRC)/common.c
	$(CC) $(CFLAGS) $(OFLAGS2) $(IFLAGS) $^ -o $@ -lm

sobel_grey: $(DIRSRC)/sobel_grey.c $(DIRSRC)/common.c
	$(CC) $(CFLAGS) $(OFLAGS2) $(IFLAGS) $^ -o $@ -lm

sobel_sqrtless: $(DIRSRC)/sobel_sqrtless.c $(DIRSRC)/common.c
	$(CC) $(CFLAGS) $(OFLAGS2) $(IFLAGS) $^ -o $@ -lm

sobel_io: $(DIRSRC)/sobel_io.c $(DIRSRC)/common.c
	$(CC) $(CFLAGS) $(OFLAGS2) $(IFLAGS) $^ -o $@ -lm

sobel_parallel: $(DIRSRC)/sobel_parallel.c $(DIRSRC)/common.c
	$(CC) $(CFLAGS) $(OFLAGS2) $(IFLAGS) $^ -o $@ -lm

clean:
	rm -Rf *~ sobel_* 
