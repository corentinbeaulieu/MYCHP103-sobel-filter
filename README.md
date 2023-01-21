# Parallel Architecture Project 

This repository contains the work asked for the Parallel Architecture Project on a sobel filter.

The *report.pdf* and all the code correspond to the first part which consist of a optimisation of a sobel filtering video processing application.
All the different changed and results can be find in the report called *report.pdf* (written in french).

The *paper.pdf* and *summary.pdf* (written in french) correspond to the second part, a shot summary of a scientifical paper about a custom extended sobel filter.

## Usage

The *measure.sh* script is the one used to get the data presented in the report. It compiles and runs 10 times each version of the application and finally plots the data using the gnuplot scripts contains in *plot*.

For a more conventional use, please compile 

```sh
$ make [version]
```

Then the usage is 

```sh
# convert a video (the test one is in the in directory)
./cvt_vid.sh v2r input_video output_raw_file

# run the sobel application
./sobel_version input_raw_file output_raw_file

# convert the raw file in mp4 video
./cvt_vid.sh r2v input_raw_file output_video
```

## Credit

The baseline code has been written by Yaspr. The changes operated are recensed in the report.
