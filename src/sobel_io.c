/*
   This code performs edge detection using a Sobel filter on a video stream meant as input to a neural network
   */
#define _LARGEFILE64_SOURCE
#include <time.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

//
#include "common.h"

//Convert an image to its grayscale equivalent - better color precision
u8 *grayscale_weighted(u8 *restrict frame, u64 nbFrames)
{
    f32 gray;

    u8 *oframe = aligned_alloc(32, W * H * sizeof(u8) * nbFrames);
    if(oframe == NULL) {
        perror("grayscale aligned_alloc : ");
        return oframe;
    }

    for(u64 i = 0; i < nbFrames; i++) {
        for (u64 j = 0; j < H * W * 3; j += 3)
        {
            //Convert RGB color values into grayscale for each pixel using color weights
            //Other possible weights: 0.59, 0.30, 0.11
            //Principle: mix different quantities of R, G, B to create a variant of gray
            gray = ((float)frame[i * W * H * 3 + j] * 0.299) + ((float)frame[i * W * H * 3 + j + 1] * 0.587) + ((float)frame[i * W * H * 3 + j + 2] * 0.114);

            oframe[i * W * H + j/3] = (u8) gray;
        }
    }

    return oframe;
}

//Convert an image to its grayscale equivalent - bad color precision
void grayscale_sampled(u8 *restrict frame)
{
    for (u64 i = 0; i < H * W * 3; i += 3)
    {
        //R: light gray
        //G: medium gray
        //B: dark gray
        u8 gray = frame[i];

        frame[i]     = gray;
        frame[i + 1] = gray;
        frame[i + 2] = gray;
    }
}

//
i16 convolve_3x3_no_zero(const u8 *restrict m, const i8 *restrict f,const u64 fh,const u64 fw)
{
    i16 r = 0;

    if(fh == 3 && fw == 2) {

        for (u64 i = 0; i < fh; i++)
            for (u64 j = 0; j < fw; j++)
                r += m[INDEX(i, j*2, W)] * f[INDEX(i, j, fw)];

    }

    else if(fh == 2 && fw == 3) {
        for (u64 i = 0; i < fh; i++)
            for (u64 j = 0; j < fw; j++)
                r += m[INDEX(i*2, j, W)] * f[INDEX(i, j, fw)];

    }

    else {
        for (u64 i = 0; i < fh; i++)
            for (u64 j = 0; j < fw; j++)
                r += m[INDEX(i, j, W)] * f[INDEX(i, j, fw)];

    }

    return r;
}

i16 convolve_7x7_no_zero(const u8 *restrict m, const i8 *restrict f,const u64 fh,const u64 fw)
{
    i16 r = 0;

    if(fh == 3 && fw == 2) {

        for (u64 i = 0; i < fh; i++)
            for (u64 j = 0; j < fw; j++)
                r += m[INDEX(i*3, j*6, W)] * f[INDEX(i, j, fw)];

    }

    else if(fh == 2 && fw == 3) {
        for (u64 i = 0; i < fh; i++)
            for (u64 j = 0; j < fw; j++)
                r += m[INDEX(i*6, j*3, W)] * f[INDEX(i, j, fw)];

    }

    else {
        for (u64 i = 0; i < fh; i++)
            for (u64 j = 0; j < fw; j++)
                r += m[INDEX(i, j, W)] * f[INDEX(i, j, fw)];

    }

    return r;
}

//
void sobel_sqrtless(const u8 *restrict cframe, u8 *restrict oframe, const u16 threshold)
{
    i16 gx, gy;
    u8 res = 0;
    u32 mag = 0;

    i8 f1[6] = { -1, 1,
        -2, 2,
        -1, 1 };

    i8 f2[6] = { -1, -2, -1,
        1,  2,  1 }; 

    //
    for (u64 i = 0; i < (H - 7); i++)
        for (u64 j = 0; j < ((W) - 7); j++)
        {
            gx = convolve_7x7_no_zero(&cframe[INDEX(i, j, W)], f1, 3, 2);
            gy = convolve_7x7_no_zero(&cframe[INDEX(i, j, W)], f2, 2, 3);

            mag = (gx * gx) + (gy * gy);

            res = (mag > threshold * threshold) * 255;

            oframe[INDEX(i, j*3, W * 3)] = res;
            oframe[INDEX(i, j*3 + 1, W * 3)] = res;
            oframe[INDEX(i, j*3 + 2, W * 3)] = res;
        }
}

//
int main(int argc, char **argv)
{
    //
    if (argc < 3)
        return printf("Usage: %s [raw input file] [raw output file]\n", argv[0]), 1;

    //
    struct timespec total1, total2;

    //
    f64 elapsed = 0.0;
    f64 mib_per_s = 0.0;
    struct timespec t1, t2;
    f64 samples[MAX_SAMPLES];

    //
    u64 nb_bytes = 1, frame_count = 0, samples_count = 0;

    f64 total_elapsed;

    //
    do {

        clock_gettime(CLOCK_MONOTONIC_RAW, &total1);

        //
        u8 *cframe = NULL; 
        u8 *oframe = NULL; 
        u8 *grey_frame = NULL;

        //
        int fpi = open(argv[1], O_RDONLY | O_LARGEFILE); 

        struct stat fileStat;
        memset(&fileStat, 0, sizeof(struct stat));
        fstat(fpi, &fileStat);

        u64 treated = 0;

        int fpo = open(argv[2], O_RDWR | O_CREAT | O_LARGEFILE, 0664);

        //
        if (fpi == -1)
            return printf("Error: cannot open file '%s'\n", argv[1]), 2;

        //
        if (fpo == -1)
            return printf("Error: cannot open file '%s'\n", argv[2]), 2;

        // Avoid Bus error
        lseek(fpo, fileStat.st_size-1, SEEK_SET);
        write(fpo, "", 1);

        //Read & process video frames
        while (treated + FILE_CHUNK <= fileStat.st_size)
        {

            cframe = mmap(NULL, FILE_CHUNK, PROT_READ, MAP_PRIVATE, fpi, treated);
            if(cframe == MAP_FAILED) {
                perror("cframe mmap");
                return 3;
            }
            //
            grey_frame = grayscale_weighted(cframe, FILE_CHUNK/FRAME_SIZE);

            munmap(cframe, FILE_CHUNK);
            cframe = NULL;
            if(grey_frame == NULL) {
                close(fpi);
                close(fpo);

                return 1;
            };

            oframe = mmap(NULL, FILE_CHUNK, PROT_READ | PROT_WRITE, MAP_SHARED, fpo, treated);
            if(oframe == MAP_FAILED) {
                perror("oframe mmap");
                return 3;
            }

            for(u64 i = 0; i < FILE_CHUNK/FRAME_SIZE; i++) {
                do
                {

                    //Start 
                    clock_gettime(CLOCK_MONOTONIC_RAW, &t1);

                    sobel_sqrtless(&grey_frame[i*FRAME_SIZE/3], &oframe[i*FRAME_SIZE], 100);

                    //Stop
                    clock_gettime(CLOCK_MONOTONIC_RAW, &t2);

                    //Nano seconds
                    elapsed = (f64)(t2.tv_sec - t1.tv_sec) + (f64) (t2.tv_nsec - t1.tv_nsec) * 1e-9;

                }
                while (elapsed <= 0.0);

                //
                if(samples_count < MAX_SAMPLES)
                    samples[samples_count++] = elapsed;

                //
                frame_count++;
            }

            free(grey_frame);
            munmap(oframe, FILE_CHUNK);
            oframe = NULL;
            treated += FILE_CHUNK;
        }

        if(treated < fileStat.st_size) {

            const u64 remaining = fileStat.st_size - treated;

            cframe = mmap(NULL, remaining, PROT_READ, MAP_PRIVATE, fpi, treated);

            if(cframe == MAP_FAILED) {
                perror("cframe mmap");
                return 3;
            }
            //
            grey_frame = grayscale_weighted(cframe, remaining / FRAME_SIZE );

            munmap(cframe, remaining);
            cframe = NULL;
            if(grey_frame == NULL) {
                close(fpi);
                close(fpo);

                return 1;
            };

            oframe = mmap(NULL, remaining, PROT_READ | PROT_WRITE, MAP_SHARED, fpo, treated);
            if(oframe == MAP_FAILED) {
                perror("oframe mmap");
                return 3;
            }

            for(u64 i = 0; i < remaining / FRAME_SIZE; i++) {
                do
                {

                    //Start 
                    clock_gettime(CLOCK_MONOTONIC_RAW, &t1);

                    sobel_sqrtless(&grey_frame[i*FRAME_SIZE/3], &oframe[i*FRAME_SIZE], 100);

                    //Stop
                    clock_gettime(CLOCK_MONOTONIC_RAW, &t2);

                    //Nano seconds
                    elapsed = (f64)(t2.tv_sec - t1.tv_sec) + (f64) (t2.tv_nsec - t1.tv_nsec) * 1e-9;

                }
                while (elapsed <= 0.0);

                samples[samples_count++] = elapsed;

                //
                frame_count++;
                treated += FRAME_SIZE;
            }

            free(grey_frame);
            munmap(oframe, remaining);
            oframe = NULL;
        }

        //
        close(fpi);
        close(fpo);

        //
        clock_gettime(CLOCK_MONOTONIC_RAW, &total2);

        total_elapsed = (f64)(total2.tv_sec - total1.tv_sec) + (f64)(total2.tv_nsec - total1.tv_nsec) * 1e-9;
    } while(total_elapsed <= 0);

    //
    f64 tot_mea,tot_mib_per_s, min, max, avg, mea, dev, fps_avg;

    //
    sort(samples, samples_count);

    //
    mea = mean(samples, samples_count);

    //
    dev = stddev(samples, samples_count);

    //
    min = samples[0];
    max = samples[samples_count - 1];

    elapsed = mea;

    fps_avg = (f64)frame_count / total_elapsed;

    tot_mib_per_s = ((f64)(FRAME_SIZE << 1) * frame_count / (1024.0 * 1024.0)) / total_elapsed;

    //2 arrays (input & output)
    mib_per_s = ((f64)(FRAME_SIZE << 1) / (1024.0 * 1024.0)) / elapsed;

    //
    fprintf(stderr, "%15llu bytes; %4.9lf s; %9.3lf MiB/s; %9.3lf fps; %3.9lf s; %3.9lf s; %3.9lf s; %9.3lf MiB/s; %3.3lf %%;\n",
            (FRAME_SIZE * frame_count) << 1,
            total_elapsed,
            tot_mib_per_s,
            fps_avg,
            min,
            max,
            mea,
            mib_per_s,
            (dev * 100.0 / mea));

    return  0;
}

