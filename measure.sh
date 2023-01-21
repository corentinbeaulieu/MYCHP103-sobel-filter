
exe="sobel_baseline sobel_compilflags sobel_grey sobel_sqrtless sobel_io sobel_parallel"

nbmeasure=10

mkdir -p out

make -B

rm -f in/input.raw
./cvt_vid.sh v2r in/input.mp4 in/input.raw

printf "#%11s; %14s; %17s; %15s; %10s; %21s; %7s;\n" "name" "avg total time" "bandwidth total" "fps" "stddev tot" "bandwidth kernel" "stddev" > measures.dat

for executable in $exe; do

    echo "MEASURING "$executable

    printf "#%11s; %20s; %14s; %17s; %15s; %13s; %13s; %13s; %21s; %7s;\n" "name" "total size" "avg total time" "bandwidth total" "fps" "min" "max" "mean" "bandwidth kernel" "stddev" > $executable.dat

    # n runs
    for (( i=0; i < $nbmeasure; i=$((i+1)) )) 
    do
        rm -f out/out.raw

        printf "%11s;" $(echo -e $executable | cut -d '_' -f2) >> $executable.dat

        ./$executable in/input.raw out/output.raw 2>> $executable.dat 1>>/dev/null
    done

    # Mean computations
    compteur=0
    values=$(cat $executable.dat | tail -n +2 | cut -d ';' -f3)
    for value in $values; do
        if [ $(( compteur%2 )) -eq 0 ]
        then
            if [ $compteur -eq 0 ]
            then
                mean=$value
            else
                mean=$mean"+"$value
            fi
        fi

        compteur=$(($compteur + 1))
    done

    sum=$(echo $mean | bc)
    mean=$(echo "scale=9;"$sum"/"$nbmeasure | bc)

    # Mean Total bandwidth computations
    compteur=0
    values=$(cat $executable.dat | tail -n +2 | cut -d ';' -f4)
    for value in $values; do
        if [ $(( compteur%2 )) -eq 0 ]
        then
            if [ $compteur -eq 0 ]
            then
                tot_bw=$value
            else
                tot_bw=$tot_bw"+"$value
            fi
        fi

        compteur=$(($compteur + 1))
    done

    sum=$(echo $tot_bw | bc)
    tot_bw=$(echo "scale=3;"$sum"/"$nbmeasure | bc)

    # Mean FPS computations
    compteur=0
    values=$(cat $executable.dat | tail -n +2 | cut -d ';' -f5)
    for value in $values; do
        if [ $(( compteur%2 )) -eq 0 ]
        then
            if [ $compteur -eq 0 ]
            then
                fps=$value
            else
                fps=$fps"+"$value
            fi
        fi

        compteur=$(($compteur + 1))
    done

    sum=$(echo $fps | bc)
    fps=$(echo "scale=3;"$sum"/"$nbmeasure | bc)

    # Standard deviation computations
    compteur=0
    values=$(cat $executable.dat | tail -n +2 | cut -d ';' -f3)
    for value in $values; do
        if [ $(( compteur%2 )) -eq 0 ]
        then
            if [ $compteur -eq 0 ]
            then
                dev="( "$value"-"$mean" )*( "$value"-"$mean" )"
            else
                dev=$dev"+( "$value"-"$mean" )*( "$value"-"$mean" )"
            fi
        fi

        compteur=$(($compteur + 1))
    done

    dev=$(echo  "scale=3;sqrt( ( "$dev" ) / "$nbmeasure" ) * 100.0 / "$mean | bc)

    # Mean kernel bandwidth computations
    compteur=0
    values=$(cat $executable.dat | tail -n +2 | cut -d ';' -f9)
    for value in $values; do
        if [ $(( compteur%2 )) -eq 0 ]
        then
            if [ $compteur -eq 0 ]
            then
                k_bw=$value
            else
                k_bw=$k_bw"+"$value
            fi
        fi

        compteur=$(($compteur + 1))
    done

    sum=$(echo $k_bw | bc)
    k_bw=$(echo "scale=3;"$sum"/"$nbmeasure | bc)

    # Mean kernel stddev computations
    compteur=0
    values=$(cat $executable.dat | tail -n +2 | cut -d ';' -f10)
    for value in $values; do
        if [ $(( compteur%2 )) -eq 0 ]
        then
            if [ $compteur -eq 0 ]
            then
                k_dev=$value
            else
                k_dev=$k_dev"+"$value
            fi
        fi

        compteur=$(($compteur + 1))
    done

    sum=$(echo $k_dev | bc)
    k_dev=$(echo "scale=3;"$sum"/"$nbmeasure | bc)

    printf "%11s;" $(echo -e $executable | cut -d '_' -f2) >> measures.dat
    printf "%14s s; %17s MiB/s; %15s fps; %10s %%; %21s MiB/s; %7s %%;\n" $mean $tot_bw $fps $dev $k_bw $k_dev >> measures.dat

done

# Plot the graphs
for script in $(ls plot/*.gp)
do
    gnuplot $script
done


