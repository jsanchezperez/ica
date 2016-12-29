set term postscript enhanced color
set xlabel "{/Symbol l}"
set ylabel "Error"
set output "lambda_graphic.eps"
#set grid
set key 
set yr [0:1]


plot "graphic_lambda_0.txt" using 1:2 with lines lw 2 title "Quadratic", "graphic_lambda_1.txt" using 1:2 with lines lw 2 title "Truncated quadratic", \
     "graphic_lambda_2.txt" using 1:2 with lines lw 2 title "Geman \\& McClure", "graphic_lambda_3.txt" using 1:2 with lines lw 2 title "Lorentzian", \
     "graphic_lambda_4.txt" using 1:2 with lines lw 2 title "Charbonnier";
