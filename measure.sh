
exe="sobel_baseline sobel_compilflags sobel_grey sobel_sqrtless sobel_io sobel_parallel"

printf "#%11s; %20s; %14s; %17s; %15s; %13s; %13s; %13s; %21s; %7s;\n" "name" "total size" "total time" "bandwidth total" "fps"  "min" "max" "mean" "bandwidth kernel" "stddev" > measures.dat

make -B

rm -f in/input.raw
./cvt_vid.sh v2r in/input.mp4 in/input.raw

for executable in $exe; do

    rm -f out/out.raw

    printf "%11s;" $(echo -e $executable | cut -d '_' -f2) >> measures.dat

  ./$executable in/input.raw out/output.raw 2>> measures.dat

done

for script in $(ls plot/*.gp); do
    gnuplot $script
done

./cvt_vid.sh r2v out/output.raw out/output.mp4
