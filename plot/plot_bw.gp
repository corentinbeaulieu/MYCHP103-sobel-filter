
set terminal png size 1280,720
set output "bw_kernel.png"

set grid y

set auto x
set xlabel "Version"

set ylabel "Bande passante du kernel (Mio/s)"
set yrange [0:]

set style data histogram 
set style histogram errorbars gap 2 lw 1

unset xtics

set xtics rotate by -45 scale 0
set datafile separator ";"

set style fill solid 0.8 

set errorbars linecolor black
set bars front

set key left top

plot "measures.dat" u 6:($7/100*$6):xtic(1) notitle

