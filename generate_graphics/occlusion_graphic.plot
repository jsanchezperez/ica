set term postscript enhanced color
set xlabel "% Occlusion"
set ylabel "Error"
set output "occlusion_graphic.eps"
set key left
set yr [0:2]

plot "graphic_occlusion_0.txt" using 1:2 with lines lw 2 title "Quadratic", "graphic_occlusion_1.txt" using 1:2 with lines lw 2 title "Truncated quadratic", \
     "graphic_occlusion_2.txt" using 1:2 with lines lw 2 title "Geman \\& McClure", "graphic_occlusion_3.txt" using 1:2 with lines lw 2 title "Lorentzian", \
     "graphic_occlusion_4.txt" using 1:2 with lines lw 2 title "Charbonnier";
