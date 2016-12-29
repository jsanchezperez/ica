// This program is free software: you can use, modify and/or redistribute it
// under the terms of the simplified BSD License. You should have received a
// copy of this license along this program. If not, see
// <http://www.opensource.org/licenses/bsd-license.html>.
//
// Copyright (C) 2015, Javier Sánchez Pérez <jsanchez@ulpgc.es>
// Copyright (C) 2014, Nelson Monzón López  <nmonzon@ctim.es>
// All rights reserved.

#include "mask.h"

#include <math.h>
#include <stdio.h>

/**
  *
  * Function to compute the gradient with centered differences
  *
**/
void gradient(
    float *input,  //input image
    float *dx,           //computed x derivative
    float *dy,           //computed y derivative
    const int nx,        //image width
    const int ny         //image height
)
{
 //#pragma omp parallel
 {
    //apply the gradient to the center body of the image
    //#pragma omp for schedule(dynamic) nowait
    for(int i = 1; i < ny-1; i++)
    {
        for(int j = 1; j < nx-1; j++)
        {
            const int k = i * nx + j;
            dx[k] = 0.5*(input[k+1] - input[k-1]);
            dy[k] = 0.5*(input[k+nx] - input[k-nx]);
        }
    }

    //apply the gradient to the first and last rows
    //#pragma omp for schedule(dynamic) nowait
    for(int j = 1; j < nx-1; j++)
    {
        dx[j] = 0.5*(input[j+1] - input[j-1]);
        dy[j] = 0.5*(input[j+nx] - input[j]);

        const int k = (ny - 1) * nx + j;

        dx[k] = 0.5*(input[k+1] - input[k-1]);
        dy[k] = 0.5*(input[k] - input[k-nx]);
    }

    //apply the gradient to the first and last columns
    //#pragma omp for schedule(dynamic) nowait
    for(int i = 1; i < ny-1; i++)
    {
        const int p = i * nx;
        dx[p] = 0.5*(input[p+1] - input[p]);
        dy[p] = 0.5*(input[p+nx] - input[p-nx]);

        const int k = (i+1) * nx - 1;

        dx[k] = 0.5*(input[k] - input[k-1]);
        dy[k] = 0.5*(input[k+nx] - input[k-nx]);
    }

    //apply the gradient to the four corners
    dx[0] = 0.5*(input[1] - input[0]);
    dy[0] = 0.5*(input[nx] - input[0]);

    dx[nx-1] = 0.5*(input[nx-1] - input[nx-2]);
    dy[nx-1] = 0.5*(input[2*nx-1] - input[nx-1]);

    dx[(ny-1)*nx] = 0.5*(input[(ny-1)*nx + 1] - input[(ny-1)*nx]);
    dy[(ny-1)*nx] = 0.5*(input[(ny-1)*nx] - input[(ny-2)*nx]);

    dx[ny*nx-1] = 0.5*(input[ny*nx-1] - input[ny*nx-1-1]);
    dy[ny*nx-1] = 0.5*(input[ny*nx-1] - input[(ny-1)*nx-1]);
 }

}





/**
 *
 * Convolution with a Gaussian
 *
 */
void
gaussian (
  float *I,    //input/output image
  int xdim,     //image width
  int ydim,     //image height
  float sigma, //Gaussian sigma
  int bc,       //boundary condition
  int precision //defines the size of the window
)
{
  int i, j, k;
  
  float den = 2 * sigma * sigma;
  int size = (int) (precision * sigma) + 1;
  int bdx = xdim + size;
  int bdy = ydim + size;
  
  if (bc && size > xdim){
      printf("GaussianSmooth: sigma too large for this bc\n");
      throw 1;
  }

  //compute the coefficients of the 1D convolution kernel
  float *B = new float [size];
  for (int i = 0; i < size; i++)
    B[i] = 1 / (sigma * sqrt (2.0 * 3.1415926)) * exp (-i * i / den);

  float norm = 0;

  //normalize the 1D convolution kernel
  for (int i = 0; i < size; i++)
    norm += B[i];

  norm *= 2;
  norm -= B[0];

  for (int i = 0; i < size; i++)
    B[i] /= norm;
  
  float *R = new float[size + xdim + size]; 
  float *T = new float[size + ydim + size];
   
  //convolution of each line of the input image
   for (k = 0; k < ydim; k++)
    {
      for (i = size; i < bdx; i++) 
        R[i] = I[(k * xdim + i - size) ];
      switch (bc)
        {
        case 0: //Dirichlet boundary conditions

          for (i = 0, j = bdx; i < size; i++, j++)
            R[i] = R[j] = 0;
          break;
        case 1: //Reflecting boundary conditions
          for (i = 0, j = bdx; i < size; i++, j++)
            {
              R[i] = I[(k * xdim + size - i ) ];
              R[j] = I[(k * xdim + xdim - i - 1)  ];
            }
          break;
        case 2: //Periodic boundary conditions
          for (i = 0, j = bdx; i < size; i++, j++)
            {
              R[i] = I[(k * xdim + xdim - size + i) ];
              R[j] = I[(k * xdim + i) ];
            }
          break;
        }

      for (i = size; i < bdx; i++)
        {

          float sum = B[0] * R[i];

          for (int j = 1; j < size; j++)
            sum += B[j] * (R[i - j] + R[i + j]);

          I[(k * xdim + i - size) ] = sum;
          
        }
    }

  //convolution of each column of the input image
  for (k = 0; k < xdim; k++)
    {
      for (i = size; i < bdy; i++)
        T[i] = I[((i - size) * xdim + k) ];

      switch (bc)
        {
        case 0: // Dirichlet boundary conditions
          for (i = 0, j = bdy; i < size; i++, j++)
            T[i] = T[j] = 0;
          break;
        case 1: // Reflecting boundary conditions
          for (i = 0, j = bdy; i < size; i++, j++)
            {
              T[i] = I[((size - i) * xdim + k) ];
              T[j] = I[((ydim - i - 1) * xdim + k) ];
            }
          break;
        case 2: // Periodic boundary conditions
          for (i = 0, j = bdx; i < size; i++, j++)
            {
              T[i] = I[((ydim - size + i) * xdim + k) ];
              T[j] = I[(i * xdim + k) ];
            }
          break;
        }

      for (i = size; i < bdy; i++)
        {
          float sum = B[0] * T[i];

          for (j = 1; j < size; j++)
            sum += B[j] * (T[i - j] + T[i + j]);

          I[((i - size) * xdim + k) ] = sum;
        }
    }
  
  delete[]B;
  delete[]R;
  delete[]T;
}
