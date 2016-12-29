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

rm graphic3_*.txt
rm graphic3_nomultiscale_*.txt

echo Parameter noise graphic

N=20

for sigma1 in {0..100}
do

  sigma=$(echo " $sigma1 * 0.3 " | bc -l)
  echo sigma=$sigma

  error0=0
  error1=0
  error2=0
  error3=0
  error4=0
  
  error00=0
  error01=0
  error02=0
  error03=0
  error04=0
  
  for ((a=0;a<N;a++))
  do
    echo -intento $a

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
      #echo $sigma $error >> graphic_$robust.txt
      
      if [ $robust == "0" ]; then
	error0=$(echo " $error0 + $error " | bc -l)
	echo "Robust 0: $error --> $error0"
      fi
      if [ $robust == "1" ]; then
	error1=$(echo " $error1 + $error " | bc -l)
	echo "Robust 1: $error --> $error1"
      fi
      if [ $robust == "2" ]; then
	error2=$(echo " $error2 + $error " | bc -l)
	echo "Robust 2: $error --> $error2"
      fi
      if [ $robust == "3" ]; then
	error3=$(echo " $error3 + $error " | bc -l)
	echo "Robust 3: $error --> $error3"
      fi
      if [ $robust == "4" ]; then
	error4=$(echo " $error4 + $error " | bc -l)
	echo "Robust 4: $error --> $error4"
      fi

      
      #graphic without multiscale
      $program1 I1.png $I2 -n 1 -f transform.mat -t 6 -r $robust -l $lambda -v -i temp.mat > tmp.txt
      $program2 I1.png $I2 transform.mat temp.mat $robust $lambda >> tmp.txt
      error=`grep "d(Hx,H'x)" tmp.txt |cut -d'=' -f 2`
      #echo $sigma $error >> graphic_$robust.txt
      
      if [ $robust == "0" ]; then
	error00=$(echo " $error00 + $error " | bc -l)
	echo "Robust 0: $error --> $error00"
      fi
      if [ $robust == "1" ]; then
	error01=$(echo " $error01 + $error " | bc -l)
	echo "Robust 1: $error --> $error01"
      fi
      if [ $robust == "2" ]; then
	error02=$(echo " $error02 + $error " | bc -l)
	echo "Robust 2: $error --> $error02"
      fi
      if [ $robust == "3" ]; then
	error03=$(echo " $error03 + $error " | bc -l)
	echo "Robust 3: $error --> $error03"
      fi
      if [ $robust == "4" ]; then
	error04=$(echo " $error04 + $error " | bc -l)
	echo "Robust 4: $error --> $error04"
      fi

      
    done

  done
  error0=$(echo "$error0/$N" | bc -l)
  error1=$(echo "$error1/$N" | bc -l)
  error2=$(echo "$error2/$N" | bc -l)
  error3=$(echo "$error3/$N" | bc -l)
  error4=$(echo "$error4/$N" | bc -l)
  
  echo $sigma $error0 >> graphic3_0.txt
  echo $sigma $error1 >> graphic3_1.txt
  echo $sigma $error2 >> graphic3_2.txt
  echo $sigma $error3 >> graphic3_3.txt
  echo $sigma $error4 >> graphic3_4.txt
  
  error00=$(echo "$error00/$N" | bc -l)
  error01=$(echo "$error01/$N" | bc -l)
  error02=$(echo "$error02/$N" | bc -l)
  error03=$(echo "$error03/$N" | bc -l)
  error04=$(echo "$error04/$N" | bc -l)
  
  echo $sigma $error00 >> graphic3_nomultiscale_0.txt
  echo $sigma $error01 >> graphic3_nomultiscale_1.txt
  echo $sigma $error02 >> graphic3_nomultiscale_2.txt
  echo $sigma $error03 >> graphic3_nomultiscale_3.txt
  echo $sigma $error04 >> graphic3_nomultiscale_4.txt
  
done

gnuplot parameter_noise_graphic3.plot
ps2pdf parameter_noise_graphic3.eps

gnuplot parameter_noise_graphic3_nomultiscale.plot
ps2pdf parameter_noise_graphic3_nomultiscale.eps
