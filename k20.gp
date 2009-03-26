#!/usr/bin/env gnuplot
set xlabel 'Seconds'
set ylabel 'dB'
set ytics nomirror
set y2tics
set y2label 'dB FS'
set yrange [-83:20]
set y2range [-103:0]
set ytics 10 add (4, 0, -24)
set grid
plot \
  'k20.dat' u 1:3 t 'Peak' w lines 2, \
  'k20.dat' u 1:2 t 'RMS' w lines 1
