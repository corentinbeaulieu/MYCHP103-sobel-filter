
set terminal png size 1280,720
set output "total_time.png"

set grid y

set auto x
set xlabel "Version"

set ylabel "Temps (s)"
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

plot "measures.dat" u 2:($5/100*$2):xtic(1) notitle

