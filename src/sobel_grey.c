/*
   This code performs edge detection using a Sobel filter on a video stream meant as input to a neural network
   */
#include <time.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <emmintrin.h>

//
#include "common.h"

//Convert an image to its grayscale equivalent - better color precision
u8 *grayscale_weighted(u8 *frame)
{
    f32 gray;

    u8 *oframe = aligned_alloc(32, W * H * sizeof(u8));
    if(oframe == NULL) {
        perror("grayscale aligned_alloc : ");
        return oframe;
    }

    for (u64 i = 0; i < H * W * 3; i += 3)
    {
        //Convert RGB color values into grayscale for each pixel using color weights
        //Other possible weights: 0.59, 0.30, 0.11
        //Principle: mix different quantities of R, G, B to create a variant of gray
        gray = ((float)frame[i] * 0.299) + ((float)frame[i + 1] * 0.587) + ((float)frame[i + 2] * 0.114);

        oframe[i/3] = (u8) gray;
    }

    return oframe;
}

//Convert an image to its grayscale equivalent - bad color precision
void grayscale_sampled(u8 *frame)
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
i32 convolve_grey(u8 *m, i32 *f, u64 fh, u64 fw)
{
    i32 r = 0;

    for (u64 i = 0; i < fh; i++)
        for (u64 j = 0; j < fw; j++)
            r += m[INDEX(i, j, W)] * f[INDEX(i, j, fw)];

    return r;
}

//
void sobel_grey(u8 *cframe, u8 *oframe, f32 threshold)
{
    i32 gx, gy;
    f32 mag = 0.0;

    i32 f1[9] = { -1, 0, 1,
        -2, 0, 2,
        -1, 0, 1 }; //3x3 matrix

    i32 f2[9] = { -1, -2, -1,
        0, 0, 0,
        1, 2, 1 }; //3x3 matrix

    //
    for (u64 i = 0; i < (H - 3); i++)
        for (u64 j = 0; j < ((W) - 3); j++)
        {
            gx = convolve_grey(&cframe[INDEX(i, j, W)], f1, 3, 3);
            gy = convolve_grey(&cframe[INDEX(i, j, W)], f2, 3, 3);

            mag = sqrt((gx * gx) + (gy * gy));

            oframe[INDEX(i, j*3, W * 3)] = (mag > threshold) ? 255 : mag;
            oframe[INDEX(i, j*3 + 1, W * 3)] = (mag > threshold) ? 255 : mag;
            oframe[INDEX(i, j*3 + 2, W * 3)] = (mag > threshold) ? 255 : mag;
        }
}

//
int main(int argc, char **argv)
{
    //
    if (argc < 3)
        return printf("Usage: %s [raw input file] [raw output file]\n", argv[0]), 1;

    struct timespec total1, total2;
    clock_gettime(CLOCK_MONOTONIC_RAW, &total1);
    //Size of a frame
    u64 size = sizeof(u8) * H * W * 3;

    //
    f64 elapsed = 0.0;
    f64 mib_per_s = 0.0;
    struct timespec t1, t2;
    f64 samples[MAX_SAMPLES];
    //
    u64 nb_bytes = 1, frame_count = 0, samples_count = 0;

    //
    u8 *cframe = _mm_malloc(size, 32);
    u8 *oframe = _mm_malloc(size, 32);
    u8 *grey_frame = NULL;

    //
    FILE *fpi = fopen(argv[1], "rb"); 
    FILE *fpo = fopen(argv[2], "wb");

    //
    if (!fpi)
        return printf("Error: cannot open file '%s'\n", argv[1]), 2;

    //
    if (!fpo)
        return printf("Error: cannot open file '%s'\n", argv[2]), 2;

    //Read & process video frames
    while ((nb_bytes = fread(cframe, sizeof(u8), H * W * 3, fpi)))
    {
        //
        grey_frame = grayscale_weighted(cframe);
        if(grey_frame == NULL) {
            _mm_free(cframe);
            _mm_free(oframe);

            //
            fclose(fpi);
            fclose(fpo);

            return 1;
        };

        do
        {

            //Start 
            clock_gettime(CLOCK_MONOTONIC_RAW, &t1);

            sobel_grey(grey_frame, oframe, 100.0);

            //Stop
            clock_gettime(CLOCK_MONOTONIC_RAW, &t2);

            //Nano seconds
            elapsed = (f64)(t2.tv_sec - t1.tv_sec) + (f64) (t2.tv_nsec - t1.tv_nsec) * 1e-9;

        }
        while (elapsed <= 0.0);

        //Seconds

        //2 arrays
        mib_per_s = ((f64)(nb_bytes << 1) / (1024.0 * 1024.0)) / elapsed;

        //
        if (samples_count < MAX_SAMPLES)
            samples[samples_count++] = elapsed;

        //frame number; size in Bytes; elapsed ns; elapsed s; bytes per second
        fprintf(stdout, "%20llu; %20llu bytes; %3.9lf s; %15.3lf MiB/s\n", frame_count, nb_bytes << 1, elapsed, mib_per_s);

        // Write this frame to the output pipe
        fwrite(oframe, sizeof(u8), H * W * 3, fpo);

        //
        frame_count++;

        free(grey_frame);
    }

    //
    _mm_free(cframe);
    _mm_free(oframe);

    //
    fclose(fpi);
    fclose(fpo);

    //
    sort(samples, samples_count);

    clock_gettime(CLOCK_MONOTONIC_RAW, &total2);

    //
    f64 total_elapsed, mib_per_s_total,min, max, avg, mea, dev, fps_avg;
    total_elapsed = (f64)(total2.tv_sec - total1.tv_sec) + (f64)(total2.tv_nsec - total1.tv_nsec) * 1e-9;

    //
    mea = mean(samples, samples_count);

    //
    dev = stddev(samples, samples_count);

    //
    min = samples[0];
    max = samples[samples_count - 1];

    elapsed = mea;

    fps_avg = 0.0;
    for(i32 i = 0; i < frame_count; i++)
        fps_avg += samples[i];

    fps_avg = (f64)frame_count / fps_avg;

    mib_per_s_total = ((f64)(size << 1) * frame_count / (1024.0 * 1024.0))     / total_elapsed;

    //2 arrays (input & output)
    mib_per_s = ((f64)(size << 1) / (1024.0 * 1024.0)) / elapsed;

    //
    fprintf(stderr, "\n%15llu bytes; %4.9lf s; %9.3lf MiB/s; %3.9lf s; %3.9lf s; %3.9lf s; %9.3lf fps; %9.3lf avg MiB/s; %3.3lf %%;\n",
            ((sizeof(u8) * H * W * 3) << 1) * frame_count,
            total_elapsed,
            mib_per_s_total,
            min,
            max,
            mea,
            fps_avg,
            mib_per_s,
            (dev * 100.0 / mea));

    return  0;
}
