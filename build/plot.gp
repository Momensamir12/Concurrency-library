set terminal pngcairo size 1200,500 enhanced font 'Sans,11'
set output 'benchmark.png'
set multiplot layout 1,2
set title 'Execution Time vs Elements'
set xlabel 'Number of Elements'
set ylabel 'Time (us)'
set logscale x
set logscale y
set grid
set key top left
plot 'bench.dat' using 1:2 with linespoints lw 2 pt 7 title 'Serial', \
     'bench.dat' using 1:3 with linespoints lw 2 pt 5 title 'Parallel'
unset logscale y
set title 'Speedup vs Elements'
set xlabel 'Number of Elements'
set ylabel 'Speedup (serial / parallel)'
set yrange [0:*]
set key top left
plot 'bench.dat' using 1:4 with linespoints lw 2 pt 7 lc rgb '#e74c3c' title 'Speedup', \
     1 with lines lw 1 lc rgb '#888888' dashtype 2 title 'Baseline (1x)'
unset multiplot
