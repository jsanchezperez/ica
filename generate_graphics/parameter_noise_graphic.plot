set term postscript enhanced color
set xlabel "{/Symbol t}"
set ylabel "Error"
set output "parameter_noise_graphic.eps"
#set grid
set key bottom
#set yr [0:3]

plot "graphic_0.txt" using 1:2 with lines lw 2 title "Quadratic", "graphic_1.txt" using 1:2 with lines lw 2 title "Truncated quadratic", \
     "graphic_2.txt" using 1:2 with lines lw 2 title "Geman \\& McClure", "graphic_3.txt" using 1:2 with lines lw 2 title "Lorentzian", \
     "graphic_4.txt" using 1:2 with lines lw 2 title "Charbonnier";