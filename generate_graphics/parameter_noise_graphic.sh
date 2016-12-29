#!/bin/bash
I1=../input/identity1.png
I2=../input/identity2.png
p=../input/identity.mat

program1=inverse_compositional_algorithm
program2=../bin/generate_output

x1=0
y1=0
x2=500
y2=0
x3=500
y3=500 
x4=0
y4=500

rm graphic_*.txt

xp1=$x1
yp1=$y1
xp2=$x2
yp2=$y2
xp3=$x3
yp3=$y3
xp4=$x4
yp4=$y4

echo Parameter noise graphic

for sigma in {0..100}
do

echo sigma=$sigma


    H=`homography_from_4points $x1 $y1 $x2 $y2 $x3 $y3 $x4 $y4 $xp1 $yp1 $xp2 $yp2 $xp3 $yp3 $xp4 $yp4`
    echo $H
    
    sigma1=2 #$(echo $sigma/2|bc -l)
    #add noise to the pointsparameters
    xp1=`add_parameter_noise $xp1 $sigma1 1`
    yp1=`add_parameter_noise $yp1 $sigma1 1`
    xp2=`add_parameter_noise $xp2 $sigma1 1`
    yp2=`add_parameter_noise $yp2 $sigma1 1`
    xp3=`add_parameter_noise $xp3 $sigma1 1`
    yp3=`add_parameter_noise $yp3 $sigma1 1`
    xp4=`add_parameter_noise $xp4 $sigma1 1`
    yp4=`add_parameter_noise $yp4 $sigma1 1`
    
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
      echo $sigma $error >> graphic_$robust.txt
      
    done  
done

gnuplot parameter_noise_graphic.plot
ps2pdf parameter_noise_graphic.eps
