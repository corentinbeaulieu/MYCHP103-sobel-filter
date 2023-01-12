# Intro
-------

  This code applies a sobel filter on a video stream.
  It processes the video frame by frame and writes the new
  frame straight to the disk drive.

# Compilation
-------------

  $ make

# Running
---------

  First, you need to convert the input video into the raw RGB format
  using the following script (make sure ffmpeg is installed on your linux distribution):

  $ ./cvt_vid.sh v2r in/input.mp4 in/input.raw

  Then you can run the program as follows:
  
  $ ./sobel in/input.raw out/output.raw

  Once the program is done, you can check the output video by converting it from the raw
  format to mp4 using the provided script as follows:

  $ ./cvt_vid.sh r2v out/output.raw out/output.mp4

  After the convertion is finished, you can play the video to verify is the output is valid
  using mplayer:
  
  $ mplayer out/output.mp4 

  An output reference mp4 video is provided.
  
# Output
--------

  The program output the frame id, the size of the frame, the run time in ns and the bandwidth in MiB/s.
  Then, it prints simple stat: min, max, average bandwidth and the standard deviation of the collected samples. 

       .
       .
       .
      358;              5529600 bytes;    47927046.000 ns;         110.031 MiB/s
      359;              5529600 bytes;    47966936.000 ns;         109.939 MiB/s

  5529600 bytes;    47454050.000 ns;    55694524.000 ns;    47954474.858 ns;         109.968 MiB/s;           1.662 %;

# Potential optimizations
-------------------------

 1 - Memory alignment.
 2 - Removing costly instructions: sqrt, division.
 3 - Loop unrolling.  
 4 - Vectorization (SSE, AVX, or AVX512 for x86_64 architectures).
   4.1 - Compiler auto-vectorization
   4.2 - OpenMP vectorization directive
   4.3 - Intrinsics (https://www.intel.com/content/www/us/en/docs/intrinsics-guide/index.html#)
   4.4 - Inline assembly :)
   
 5 - Parallelization using OpenMP on the outer-most loop.
 6 - Use different compilers (at least two): gcc, clang, aocc, icx, and icc
 7 - Add other performance metrics: FPS (Frames Per Second), elapsed time, ...
 8 - Bufferize I/O (load & process more than one frame at a time).
 
# Report
--------

 You are to provide a performance analysis report of all the versions with plots comparing the
 performance of all the conducted experiments.