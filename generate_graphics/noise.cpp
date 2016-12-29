#include <unistd.h>
#include <math.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>

#include "mt19937ar.h"

double add_noise(double u, double sigma) 
{
  mt_init_genrand((unsigned long int) time (NULL) + (unsigned long int) getpid());

  double a=mt_genrand_res53();
  double b=mt_genrand_res53();
  double z = (sigma)*sqrt(-2.0*log(a))*cos(2.0*M_PI*b);

  return u + z;
}


int main (int argc, char *argv[])
{
  if(argc<3)
    printf("\n\n<Usage>: %s input_param sigma\n\n", argv[0]); 
  else
    printf("%f\n", add_noise(atof(argv[1]),atof(argv[2])));
    
}
