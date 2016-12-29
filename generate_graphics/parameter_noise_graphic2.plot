set term postscript enhanced color 
set xlabel "{/Symbol t}"
set ylabel "Error"
set output "parameter_noise_graphic2.eps"
#set grid
set key bottom
set yr [0:20]

plot "graphic2_0.txt" using 1:2 with lines lw 2 title "Quadratic", "graphic2_1.txt" using 1:2 with lines lw 2 title "Truncated quadratic", \
     "graphic2_2.txt" using 1:2 with lines lw 2 title "Geman \\& McClure", "graphic2_3.txt" using 1:2 with lines lw 2 title "Lorentzian", \
     "graphic2_4.txt" using 1:2 with lines lw 2 title "Charbonnier";