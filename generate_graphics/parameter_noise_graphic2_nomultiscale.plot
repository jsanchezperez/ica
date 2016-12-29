set term postscript enhanced color 
set xlabel "{/Symbol t}"
set ylabel "Error"
set output "parameter_noise_graphic2_nomultiscale.eps"
#set grid
set key bottom
set yr [0:20]

plot "graphic2_nomultiscale_0.txt" using 1:2 with lines lw 2 title "Quadratic", "graphic2_nomultiscale_1.txt" using 1:2 with lines lw 2 title "Truncated quadratic", \
     "graphic2_nomultiscale_2.txt" using 1:2 with lines lw 2 title "Geman \\& McClure", "graphic2_nomultiscale_3.txt" using 1:2 with lines lw 2 title "Lorentzian", \
     "graphic2_nomultiscale_4.txt" using 1:2 with lines lw 2 title "Charbonnier";