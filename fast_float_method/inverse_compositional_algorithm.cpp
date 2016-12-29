// This program is free software: you can use, modify and/or redistribute it
// under the terms of the simplified BSD License. You should have received a
// copy of this license along this program. If not, see
// <http://www.opensource.org/licenses/bsd-license.html>.
//
// Copyright (C) 2015, Javier Sánchez Pérez <jsanchez@dis.ulpgc.es>
// All rights reserved.

/** 
  * 
  *  This code implements the 'inverse compositional algorithm' proposed in
  *     [1] S. Baker, and I. Matthews. (2004). Lucas-kanade 20 years on: A 
  *         unifying framework. International Journal of Computer Vision, 
  *         56(3), 221-255.
  *     [2] S. Baker, R. Gross, I. Matthews, and T. Ishikawa. (2004). 
  *         Lucas-kanade 20 years on: A unifying framework: Part 2. 
  *         International Journal of Computer Vision, 56(3), 221-255.
  *  
  *  This implementation is for color images. It calculates the global 
  *  transform between two images. It uses robust error functions and a 
  *  coarse-to-fine strategy for computing large displacements
  * 
**/

#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include <vector>

#include "bicubic_interpolation.h"
#include "inverse_compositional_algorithm.h"
#include "matrix.h"
#include "mask.h"
#include "transformation.h"
#include "zoom.h"
#include "file.h"


using namespace std;


/**
 *
 *  Derivative of robust error functions
 *
 */
float rhop(
  float t2,     //squared difference of both images  
  float lambda, //robust threshold
  int    type    //choice of the robust error function
)
{
  float result=0.0;
  float lambda2=lambda*lambda;
  switch(type)
  {
    case QUADRATIC:
      result=1;
      break;
    default: 
    case TRUNCATED_QUADRATIC:
      if(t2<lambda2) result=1.0;
      else result=0.0;
      break;  
    case GERMAN_MCCLURE:
      result=lambda2/((lambda2+t2)*(lambda2+t2));
      break;
    case LORENTZIAN: 
      result=1/(lambda2+t2);
      break;
    case CHARBONNIER:
      result=1.0/(sqrt(t2+lambda2));
      break;
  }
  return result;
}

 
/**
 *
 *  Function to compute DI^t*J
 *  from the gradient of the image and the Jacobian
 *
 */
void steepest_descent_images
(
  float *Ix,  //x derivate of the image
  float *Iy,  //y derivate of the image
  float *J,   //Jacobian matrix
  float *DIJ, //output DI^t*J
  int nparams, //number of parameters
  vector<int> &x //corner positions
)
{
#pragma omp parallel for
  for(unsigned int p=0; p<x.size(); p++)
    for(int n=0; n<nparams; n++)
      DIJ[p*nparams+n]=Ix[x[p]]*J[2*p*nparams+n]+Iy[x[p]]*J[2*p*nparams+n+nparams];
}

/**
 *
 *  Function to compute the Hessian matrix
 *  the Hessian is equal to DIJ^t*DIJ
 *
 */
void hessian
(
  float *DIJ, //the steepest descent image
  float *H,   //output Hessian matrix
  int nparams, //number of parameters
  int N        //number of values
) 
{
  //initialize the hessian to zero
#pragma omp parallel for
  for(int k=0; k<nparams*nparams; k++)
    H[k] = 0;
 
  //calculate the hessian in a neighbor window
#pragma omp parallel for
  for(int k=0; k<nparams; k++)
    for(int l=0; l<nparams; l++)
      for(int i=0; i<N; i++)
	H[k*nparams+l]+=DIJ[i*nparams+k]*DIJ[i*nparams+l];
}


/**
 *
 *  Function to compute the Hessian matrix with robust error functions
 *  the Hessian is equal to rho'*DIJ^t*DIJ
 *
 */
void hessian
(
  float *DIJ, //the steepest descent image
  float *rho, //robust function
  float *H,   //output Hessian matrix
  int nparams, //number of parameters
  int N        //number of values
) 
{
  //initialize the hessian to zero
#pragma omp parallel for
  for(int k=0; k<nparams*nparams; k++)
    H[k]=0;

  //calculate the hessian in a neighbor window
#pragma omp parallel for
  for(int k=0; k<nparams; k++)
    for(int l=0; l<nparams; l++)
      for(int i=0; i<N; i++)
	H[k*nparams+l]+=rho[i]*DIJ[i*nparams+k]*DIJ[i*nparams+l];
}



/**
 *
 *  Function to compute the inverse of the Hessian
 *
 */
void inverse_hessian
(
  float *H,   //input Hessian
  float *H_1, //output inverse Hessian 
  int nparams  //number of parameters
) 
{
  if(inverse(H, H_1, nparams)==-1) 
    //if the matrix is not invertible, set parameters to 0
    for(int i=0; i<nparams*nparams; i++) H_1[i]=0;
}


/**
 *
 *  Function to compute I2(W(x;p))-I1(x)
 *
 */
void difference_image
(
  float *I,  //first image I1(x)
  float *Iw, //second warped image I2(x'(x;p)) 
  vector<int> &x,
  float *DI  //output difference array
) 
{
#pragma omp parallel for
  for(unsigned int i=0; i<x.size(); i++)
    DI[i]=Iw[i]-I[x[i]];
}


/**
 *
 *  Function to store the values of p'((I2(x'(x;p))-I1(x))²)
 *
 */
void robust_error_function
(
  float *DI,   //input difference array
  float *rho,  //output robust function
  float lambda,//threshold used in the robust functions
  int    type,  //choice of robust error function
  int N         //number of values
)
{
#pragma omp parallel for
  for(int i=0;i<N;i++)
  {
    float norm=DI[i]*DI[i];
    rho[i]=rhop(norm,lambda,type);
  }
}


/**
 *
 *  Function to compute b=Sum(DIJ^t * DI)
 *
 */
void independent_vector
(
  float *DIJ, //the steepest descent image
  float *DI,  //I2(x'(x;p))-I1(x) 
  float *b,   //output independent vector
  int nparams, //number of parameters
  int N        //number of columns
)
{
  //initialize the vector to zero
  for(int k=0; k<nparams; k++)
    b[k]=0;

#pragma omp parallel for
  for(int k=0; k<nparams; k++)
    for(int i=0; i<N; i++)
      b[k]+=DIJ[i*nparams+k]*DI[i];
}


/**
 *
 *  Function to compute b=Sum(rho'*DIJ^t * DI)
 *  with robust error functions
 *
 */
void independent_vector
(
  float *DIJ, //the steepest descent image
  float *DI,  //I2(x'(x;p))-I1(x) 
  float *rho, //robust function
  float *b,   //output independent vector
  int nparams, //number of parameters
  int N        //number of values
)
{
  //initialize the vector to zero
  for(int k=0; k<nparams; k++)
    b[k]=0;

#pragma omp parallel for
  for(int k=0; k<nparams; k++)
    for(int i=0; i<N; i++)
      b[k]+=rho[i]*DIJ[i*nparams+k]*DI[i];
}


/**
 *
 *  Function to solve for dp
 *  
 */
float parametric_solve
(
  float *H_1, //inverse Hessian
  float *b,   //independent vector
  float *dp,  //output parameters increment 
  int nparams  //number of parameters
)
{
  float error=0.0;
  Axb(H_1, b, dp, nparams);
  for(int i=0; i<nparams; i++) error+=dp[i]*dp[i];
  return sqrt(error);
}


/**
  *
  *  Select points
  *
**/
void select_points(
  float *I,
  vector<int> &x,
  int nx,
  int ny,
  int verbose
)
{
  static int s=0;
  int radius=3;
  
  /*for(int i=2*radius;i<ny-2*radius;i+=radius*5)
    for(int j=2*radius;j<nx-2*radius;j+=radius*5)
    {
      for(int k=i-radius;k<=i+radius; k++)
	for(int l=j-radius;l<=j+radius; l++)
	  x.push_back(k*nx+l);
    }
*/

  for(int i=radius;i<ny-radius;i+=radius)
    for(int j=radius;j<nx-radius;j+=radius)
	  x.push_back(i*nx+j);

  

  if(verbose) 
  {
    float *A=new float[nx*ny]();
    printf("Number of Harris points: %ld \n", x.size());
    #pragma omp parallel for
    for(int i=0;i<nx*ny;i++) A[i]=I[i];
    #pragma omp parallel for
    for(unsigned int i=0;i<x.size();i++)
      A[x[i]]=255;
    char name[100];
    sprintf(name, "verbose_Harris_%d.png",s++);
    save_image(name, A, nx, ny, 1);
    delete[] A;
  }

  
}


/**
  *
  *  Inverse compositional algorithm
  *  Quadratic version - L2 norm
  * 
  *
**/
void inverse_compositional_algorithm(
  float *I1,   //first image
  float *I2,   //second image
  float *p,    //parameters of the transform (output)
  int nparams,  //number of parameters of the transform
  float TOL,   //Tolerance used for the convergence in the iterations
  int nx,        //number of columns
  int ny,        //number of rows
  int verbose   //enable verbose mode
)
{
  float *Ix =new float[nx*ny];   //x derivate of the first image
  float *Iy =new float[nx*ny];   //y derivate of the first image

  //Evaluate the gradient of I1
  gradient(I1, Ix, Iy, nx, ny);
  
  //find corner points
  vector<int> x;
  select_points(I1, x, nx, ny, verbose);
  

  int N=x.size();
  int size2=N*nparams;   //size of the image with transform parameters
  int size3=nparams*nparams; //size for the Hessian
  int size4=2*N*nparams; 
  float *Iw =new float[N];   //warp of the second image/
  float *DI =new float[N];   //error image (I2(w)-I1)
  float *DIJ=new float[size2];   //steepest descent images
  float *dp =new float[nparams]; //incremental solution
  float *b  =new float[nparams]; //steepest descent images
  float *J  =new float[size4];   //jacobian matrix for all points
  float *H  =new float[size3];   //Hessian matrix
  float *H_1=new float[size3];   //inverse Hessian matrix

  //Evaluate the Jacobian
  jacobian(J, x, nparams, nx);

  //Compute the steepest descent images
  steepest_descent_images(Ix, Iy, J, DIJ, nparams, x);

  //Compute the Hessian matrix
  hessian(DIJ, H, nparams, N);
  inverse_hessian(H, H_1, nparams);

  //Iterate
  float error=1E10;
  int niter=0;

  do{     
    //Warp image I2
    bicubic_interpolation(I2, x, Iw, p, nparams, nx, ny);

    //Compute the error image (I1-I2w)
    difference_image(I1, Iw, x, DI);
    
    //Compute the independent vector
    independent_vector(DIJ, DI, b, nparams, N);

    //Solve equation and compute increment of the motion 
    error=parametric_solve(H_1, b, dp, nparams);

    //Update the warp x'(x;p) := x'(x;p) * x'(x;dp)^-1
    update_transform(p, dp, nparams);

    if(verbose)
    {
      printf("|Dp|=%f: p=(",error);
      for(int i=0;i<nparams-1;i++)
        printf("%f ",p[i]);
      printf("%f)\n",p[nparams-1]);
    }
    niter++;    
  }
  while(error>TOL && niter<MAX_ITER);
  
  //delete allocated memory
  delete []DI;
  delete []Ix;
  delete []Iy;
  delete []Iw;
  delete []DIJ;
  delete []dp;
  delete []b;
  delete []J;
  delete []H;
  delete []H_1;
}



/**
  *
  *  Inverse compositional algorithm 
  *  Version with robust error functions
  * 
**/
void robust_inverse_compositional_algorithm(
  float *I1,    //first image
  float *I2,    //second image
  float *p,     //parameters of the transform (output)
  int nparams,   //number of parameters of the transform
  float TOL,    //Tolerance used for the convergence in the iterations
  int    robust, //robust error function
  float lambda, //parameter of robust error function
  int nx,        //number of columns
  int ny,        //number of rows
  int verbose    //enable verbose mode
)
{  
  float *Ix =new float[nx*ny];       //x derivate of the first image
  float *Iy =new float[nx*ny];       //y derivate of the first image

  //Evaluate the gradient of I1
  gradient(I1, Ix, Iy, nx, ny);

  //find corner points
  vector<int> x;
  select_points(I1, x, nx, ny, verbose);      

  int N=x.size();            //number of corner points
  int size2=N*nparams;   //size of the image with transform parameters
  int size3=nparams*nparams; //size for the Hessian
  int size4=2*N*nparams; 
  float *Iw =new float[N];       //warp of the second image/
  float *DI =new float[N];       //error image (I2(w)-I1)
  float *DIJ=new float[size2];   //steepest descent images
  float *dp =new float[nparams]; //incremental solution
  float *b  =new float[nparams]; //steepest descent images
  float *J  =new float[size4];   //jacobian matrix for all points
  float *H  =new float[size3];   //Hessian matrix
  float *H_1=new float[size3];   //inverse Hessian matrix
  float *rho=new float[N];       //robust function  
  
  //Evaluate the Jacobian
  jacobian(J, x, nparams, nx);

  //Compute the steepest descent images
  steepest_descent_images(Ix, Iy, J, DIJ, nparams, x);
  
  //Iterate
  float error=1E10;
  int niter=0;
  float lambda_it;
  
  if(lambda>0) lambda_it=lambda;
  else lambda_it=LAMBDA_0;
  
  do{     
    //Warp image I2
    bicubic_interpolation(I2, x, Iw, p, nparams, nx, ny);

    //Compute the error image (I1-I2w)
    difference_image(I1, Iw, x, DI);

    //compute robustifiction function
    robust_error_function(DI, rho, lambda_it, robust, N);
    if(lambda<=0 && lambda_it>LAMBDA_N) 
    {
      lambda_it*=LAMBDA_RATIO;
      if(lambda_it<LAMBDA_N) lambda_it=LAMBDA_N;
    }

    //Compute the independent vector
    independent_vector(DIJ, DI, rho, b, nparams, N);

    //Compute the Hessian matrix
    hessian(DIJ, rho, H, nparams, N);
    inverse_hessian(H, H_1, nparams);

    //Solve equation and compute increment of the motion 
    error=parametric_solve(H_1, b, dp, nparams);

    //Update the warp x'(x;p) := x'(x;p) * x'(x;dp)^-1
    update_transform(p, dp, nparams);

    if(verbose) 
    {
      printf("|Dp|=%f: p=(",error);
      for(int i=0;i<nparams-1;i++)
        printf("%f ",p[i]);
      printf("%f), lambda=%f\n",p[nparams-1],lambda_it);
    }
    niter++;    
  }
  while(error>TOL && niter<MAX_ITER);
  
  //delete allocated memory
  delete []DI;
  delete []Ix;
  delete []Iy;
  delete []Iw;
  delete []DIJ;
  delete []dp;
  delete []b;
  delete []J;
  delete []H;
  delete []H_1;
  delete []rho;
}


/**
  *
  *  Multiscale approach for computing the optical flow
  *
**/
void pyramidal_inverse_compositional_algorithm(
    float *I1,     //first image
    float *I2,     //second image
    float *p,      //parameters of the transform
    int    nparams, //number of parameters
    int    nxx,     //image width
    int    nyy,     //image height
    int    nscales, //number of scales
    float nu,      //downsampling factor
    float TOL,     //stopping criterion threshold
    int    robust,  //robust error function
    float lambda,  //parameter of robust error function
    bool   verbose  //switch on messages
)
{
    int size=nxx*nyy;

    float **I1s=new float*[nscales];
    float **I2s=new float*[nscales];
    float **ps =new float*[nscales];

    int *nx=new int[nscales];
    int *ny=new int[nscales];

    I1s[0]=new float[size];
    I2s[0]=new float[size];

    //copy the input images
    #pragma omp parallel for
    for(int i=0;i<size;i++)
    {
      I1s[0][i]=I1[i];
      I2s[0][i]=I2[i];
    }

    ps[0]=p;
    nx[0]=nxx;
    ny[0]=nyy;

    //initialization of the transformation parameters at the finest scale
    for(int i=0; i<nparams; i++)
      p[i]=0.0;

    //create the scales
    for(int s=1; s<nscales; s++)
    {
      zoom_size(nx[s-1], ny[s-1], nx[s], ny[s], nu);

      const int size=nx[s]*ny[s];

      I1s[s]=new float[size];
      I2s[s]=new float[size];
      ps[s] =new float[nparams];
      
      for(int i=0; i<nparams; i++)
        ps[s][i]=0.0;

      //zoom the images from the previous scale
      zoom_out(I1s[s-1], I1s[s], nx[s-1], ny[s-1], nu);
      zoom_out(I2s[s-1], I2s[s], nx[s-1], ny[s-1], nu);
    }  

    //pyramidal approach for computing the transformation
    for(int s=nscales-1; s>=0; s--)
    {
      if(verbose) printf("Scale: %d (%d,%d)\n",s,nx[s],ny[s]);

      //incremental refinement for this scale
      if(robust==QUADRATIC)
      {
        if(verbose) printf("(L2 norm)\n");

        inverse_compositional_algorithm(
          I1s[s], I2s[s], ps[s], nparams, TOL, nx[s], ny[s], verbose
        );
      }
      else
      {
        if(verbose) printf("(Robust error function %d)\n",robust);

        robust_inverse_compositional_algorithm(
          I1s[s], I2s[s], ps[s], nparams, TOL, 
          robust, lambda, nx[s], ny[s], verbose
        );
      }

      //if it is not the finer scale, then upsample the parameters
      if(s) 
        zoom_in_parameters(
          ps[s], ps[s-1], nparams, nx[s], ny[s], nx[s-1], ny[s-1]
        );
    }

    //delete allocated memory
    delete []I1s[0];
    delete []I2s[0];
    for(int i=1; i<nscales; i++)
    {
      delete []I1s[i];
      delete []I2s[i];
      delete []ps [i];
    }
    delete []I1s;
    delete []I2s;
    delete []ps;
    delete []nx;
    delete []ny;
}
