// This program is free software: you can use, modify and/or redistribute it
// under the terms of the simplified BSD License. You should have received a
// copy of this license along this program. If not, see
// <http://www.opensource.org/licenses/bsd-license.html>.
//
// Copyright (C) 2015, Javier Sánchez Pérez <jsanchez@dis.ulpgc.es>
// All rights reserved.

#ifndef INVERSE_COMPOSITIONAL_ALGORITHM
#define INVERSE_COMPOSITIONAL_ALGORITHM

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

#define QUADRATIC 0
#define TRUNCATED_QUADRATIC 1
#define GERMAN_MCCLURE 2
#define LORENTZIAN 3
#define CHARBONNIER 4

#define MAX_ITER 30
#define LAMBDA_0 80
#define LAMBDA_N 5
#define LAMBDA_RATIO 0.90

/**
 *
 *  Derivative of robust error functions
 *
 */
float rhop(
  float t2,    //squared difference of both images  
  float sigma, //robust threshold
  int    type   //choice of the robust error function
);
 

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
  int nx,       //number of columns of the image
  int ny,       //number of rows of the image
  float TOL,   //Tolerance used for the convergence in the iterations
  int verbose=0 //enable verbose mode
);


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
  int nx,        //number of columns of the image
  int ny,        //number of rows of the image
  float TOL,    //Tolerance used for the convergence in the iterations
  int    robust, //robust error function
  float lambda, //parameter of robust error function
  int verbose=0  //enable verbose mode
);

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
);

#endif
