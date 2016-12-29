set term postscript enhanced color
set xlabel "{/Symbol s}"
set ylabel "Error"
set output "parameter_noise_graphic3_nomultiscale.eps"
#set grid
set key bottom
set yr [0:20]
set xr [0:30]

plot "graphic3_nomultiscale_0.txt" using 1:2 with lines lw 2 title "Quadratic", "graphic3_nomultiscale_1.txt" using 1:2 with lines lw 2 title "Truncated quadratic", \
     "graphic3_nomultiscale_2.txt" using 1:2 with lines lw 2 title "Geman \\& McClure", "graphic3_nomultiscale_3.txt" using 1:2 with lines lw 2 title "Lorentzian", \
     "graphic3_nomultiscale_4.txt" using 1:2 with lines lw 2 title "Charbonnier";