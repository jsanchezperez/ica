I1=../input/zoom_rotation1.png
I2=../input/zoom_rotation2.png
p=../input/zoom_rotation.mat

program1=../bin/inverse_compositional_algorithm
program2=../bin/generate_output

rm graphic_noise_*.txt

N=20

for sigma in {0..100}
do
  echo sigma=$sigma

  error0=0
  error1=0
  error2=0
  error3=0
  error4=0
  
  for ((a=0;a<N;a++))
  do
      echo -intento $a
      ../bin/add_noise $I1 I1.png $sigma   
      ../bin/add_noise $I2 I2.png $sigma   

      for robust in {0..4}
      do
	#if [ $robust -eq 4 ]; then 
	#  lambda=1
	#else
	  lambda=0
	#fi

	$program1 I1.png I2.png -f transform.mat -t 4 -r $robust -l $lambda -v > tmp.txt
	$program2 I1.png I2.png transform.mat $p $robust $lambda >> tmp.txt

	error=`grep "d(Hx,H'x)" tmp.txt |cut -d'=' -f 2`
	
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
      done

  done
  error0=$(echo "$error0/$N" | bc -l)
  error1=$(echo "$error1/$N" | bc -l)
  error2=$(echo "$error2/$N" | bc -l)
  error3=$(echo "$error3/$N" | bc -l)
  error4=$(echo "$error4/$N" | bc -l)
  
  echo $sigma $error0 >> graphic_noise_0.txt
  echo $sigma $error1 >> graphic_noise_1.txt
  echo $sigma $error2 >> graphic_noise_2.txt
  echo $sigma $error3 >> graphic_noise_3.txt
  echo $sigma $error4 >> graphic_noise_4.txt
done

gnuplot noise_graphic.plot
ps2pdf noise_graphic.eps