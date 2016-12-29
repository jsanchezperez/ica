#!/bin/bash
I1=../../ipol_demo/app/inverse_compositional_algorithm/input/identity1.png
I2=../../ipol_demo/app/inverse_compositional_algorithm/input/identity2.png
p=../../ipol_demo/app/inverse_compositional_algorithm/input/identity.mat

program1=inverse_compositional_algorithm
program2=../../ipol_demo/app/inverse_compositional_algorithm/bin/generate_output

x1=0
y1=0
x2=500
y2=0
x3=500
y3=500 
x4=0
y4=500

rm graphic2_*.txt
rm graphic2_nomultiscale_*.txt

sigma1=50 #$(echo $sigma/2|bc -l)
xp1=`add_parameter_noise $x1 $sigma1 1`
yp1=`add_parameter_noise $y1 $sigma1 1`
xp2=`add_parameter_noise $x2 $sigma1 1`
yp2=`add_parameter_noise $y2 $sigma1 1`
xp3=`add_parameter_noise $x3 $sigma1 1`
yp3=`add_parameter_noise $y3 $sigma1 1`
xp4=`add_parameter_noise $x4 $sigma1 1`
yp4=`add_parameter_noise $y4 $sigma1 1`

echo parameter_noise_graphic 2
echo parameter_noise_graphic 2 no multiscale


for sigma in {0..100}
do

  echo sigma=$sigma

  xpp1=$(echo "(100 - $sigma )/100 * $x1 + $sigma /100* $xp1" |bc -l)
  ypp1=$(echo "(100 - $sigma )/100 * $y1 + $sigma /100* $yp1" |bc -l)
  xpp2=$(echo "(100 - $sigma )/100 * $x2 + $sigma /100* $xp2" |bc -l)
  ypp2=$(echo "(100 - $sigma )/100 * $y2 + $sigma /100* $yp2" |bc -l)
  xpp3=$(echo "(100 - $sigma )/100 * $x3 + $sigma /100* $xp3" |bc -l)
  ypp3=$(echo "(100 - $sigma )/100 * $y3 + $sigma /100* $yp3" |bc -l)
  xpp4=$(echo "(100 - $sigma )/100 * $x4 + $sigma /100* $xp4" |bc -l)
  ypp4=$(echo "(100 - $sigma )/100 * $y4 + $sigma /100* $yp4" |bc -l)

  H=`homography_from_4points $x1 $y1 $x2 $y2 $x3 $y3 $x4 $y4 $xpp1 $ypp1 $xpp2 $ypp2 $xpp3 $ypp3 $xpp4 $ypp4`
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
  parametric_warp $I2 I1.png temp.mat 1

  for robust in {0..4}
  do
    #if [ $robust -eq 4 ]; then 
    #  lambda=1
    #else
      lambda=0
    #fi
    
    $program1 I1.png $I2 -f transform.mat -t 6 -r $robust -l $lambda -v -i temp.mat > tmp.txt
    $program2 I1.png $I2 transform.mat temp.mat $robust $lambda >> tmp.txt
    error=`grep "d(Hx,H'x)" tmp.txt |cut -d'=' -f 2`
    echo $sigma $error >> graphic2_$robust.txt
    
    $program1 I1.png $I2 -n 1 -f transform.mat -t 6 -r $robust -l $lambda -v -i temp.mat > tmp.txt
    $program2 I1.png $I2 transform.mat temp.mat $robust $lambda >> tmp.txt
    error=`grep "d(Hx,H'x)" tmp.txt |cut -d'=' -f 2`
    echo $sigma $error >> graphic2_nomultiscale_$robust.txt

  done
done

gnuplot parameter_noise_graphic2.plot
ps2pdf parameter_noise_graphic2.eps

gnuplot parameter_noise_graphic2_nomultiscale.plot
ps2pdf parameter_noise_graphic2_nomultiscale.eps
