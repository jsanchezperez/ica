set term postscript enhanced color
set xlabel "{/Symbol s}"
set ylabel "Error"
set output "noise_graphic.eps"
set key left
#set yr [0:1.5]

plot "graphic_noise_0.txt" using 1:2 with lines lw 2 title "Quadratic", "graphic_noise_1.txt" using 1:2 with lines lw 2 title "Truncated quadratic", \
     "graphic_noise_2.txt" using 1:2 with lines lw 2 title "Geman \\& McClure", "graphic_noise_3.txt" using 1:2 with lines lw 2 title "Lorentzian", \
     "graphic_noise_4.txt" using 1:2 with lines lw 2 title "Charbonnier";