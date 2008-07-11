#!/usr/bin/env gnuplot
set xlabel 'Seconds'
set ylabel 'dB'
set ytics nomirror
set y2tics
set y2label 'dB FS'
set y2range [-90:0]
set yrange [-70:20]

plot \
  'k20.dat' u 1:2 t 'RMS' w lines, \
  '' u 1:3 t 'Peak' w dots
# '' u 1:4 t 'Max Peak' w lines
