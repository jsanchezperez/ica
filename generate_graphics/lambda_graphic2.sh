#!/bin/bash
I1=../input/identity1.png
I2=../input/identity2.png
p=../input/identity.mat

program1=../bin/inverse_compositional_algorithm
program2=../bin/generate_output

x1=0
y1=0
x2=500
y2=0
x3=500
y3=500 
x4=0
y4=500

rm graphic_lambda_*.txt

sigma=30 #$(echo $sigma/2|bc -l)
#add noise to the pointsparameters
xp1=`add_parameter_noise $x1 $sigma 1`
yp1=`add_parameter_noise $y1 $sigma 1`
xp2=`add_parameter_noise $x2 $sigma 1`
yp2=`add_parameter_noise $y2 $sigma 1`
xp3=`add_parameter_noise $x3 $sigma 1`
yp3=`add_parameter_noise $y3 $sigma 1`
xp4=`add_parameter_noise $x4 $sigma 1`
yp4=`add_parameter_noise $y4 $sigma 1`

H=`homography_from_4points $x1 $y1 $x2 $y2 $x3 $y3 $x4 $y4 $xp1 $yp1 $xp2 $yp2 $xp3 $yp3 $xp4 $yp4`
echo $H

h1=$(echo $H |cut -d' ' -f 1)
h1=$(echo $h1-1|bc)
h2=$(echo $H |cut -d' ' -f 2)
h3=$(echo $H |cut -d' ' -f 3)
h4=$(echo $H |cut -d' ' -f 4)
h5=$(echo $H |cut -d' ' -f 5)
h5=$(echo $h5-1|bc)
h6=$(echo $H |cut -d' ' -f 6)
h7=$(echo $H |cut -d' ' -f 7)
h8=$(echo $H |cut -d' ' -f 8)

echo 8 > temp.mat
echo $h1 $h2 $h3 $h4 $h5 $h6 $h7 $h8 >> temp.mat

#warp images
parametric_warp $I2 I11.png temp.mat 1

rm graphic_lambda_*.txt

for lambda in {1..100}
do

  for robust in {0..4}
  do
    ../bin/add_noise I11.png I1.png 50
    ../bin/add_noise $I2 I2.png 50
    
    $program1 I1.png $I2 -f transform.mat -t 8 -r $robust -l $lambda -v  > tmp.txt
    $program2 I1.png $I2 transform.mat temp.mat $robust $lambda >> tmp.txt
    error=`grep "d(Hx,H'x)" tmp.txt |cut -d'=' -f 2`
    echo $lambda $error >> graphic_lambda_$robust.txt
  done
done

gnuplot lambda_graphic.plot
ps2pdf lambda_graphic.eps lambda_graphic.pdf
