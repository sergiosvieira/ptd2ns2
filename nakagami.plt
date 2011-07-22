set terminal postscript eps color "ArialMT" 20
set encoding iso_8859_1

set grid ytics lt 0 lw 2 lc rgb "#cccccc"
set grid xtics lt 0 lw 2 lc rgb "#cccccc"

set style line 1 lt 1 lw 1 lc 1 
set style line 2 lt 2 lw 3 lc 7
set style line 3 lt 1 lw 1 lc 3
set style line 4 lt 2 lw 1 lc 1
set pointsize 1.5 

set xrange [0:1000]
set xtics 100

set yrange [0:1]
set ytics 0.25

set xlabel "Distância"
set ylabel "Probabilidade de Recepção"
set key box width 2
set key right top

set output 'nakagami.eps'

plot    "data.dat" using 1:2:3 title "Nakagami" smooth csplines with lines ls 1
