set xlabel 'Seconds'
set ylabel 'dB'
plot [:] [-80:20]  \
'k20.dat' u 1:2 t 'RMS' w lines, \
'' u 1:3 t 'Peak' w points, \
'' u 1:4 t 'Max Peak' w lines
