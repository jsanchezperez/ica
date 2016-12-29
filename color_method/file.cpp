// This program is free software: you can use, modify and/or redistribute it
// under the terms of the simplified BSD License. You should have received a
// copy of this license along this program. If not, see
// <http://www.opensource.org/licenses/bsd-license.html>.
//
// Copyright (C) 2015, Javier Sánchez Pérez <jsanchez@ulpgc.es>
// All rights reserved.


#include <stdio.h>

#include "file.h"

extern "C"
{
#include "iio.h"
}


/**
 *
 *  Functions to read images using the iio library
 *  It allocates memory for the image and returns true if it
 *  correctly reads the image
 * 
 */
bool read_image
(
  char *fname,  //file name
  double **f,   //output image
  int &nx,      //number of columns of the image
  int &ny,      //number of rows of the image
  int &nz       //number of channels of the image
)
{
  *f = iio_read_image_double_vec(fname, &nx, &ny, &nz);
  return *f ? true : false;
}

bool read_image
(
  char *fname,  //file name
  float **f,    //output image
  int &nx,      //number of columns of the image
  int &ny,      //number of rows of the image
  int &nz       //number of channels of the image
)
{
  *f = iio_read_image_float_vec(fname, &nx, &ny, &nz);
  return *f ? true : false;
}

/**
 *
 *  Functions to save images using the iio library
 *
 */
void save_image
(
  char *fname,  //file name
  double *f,    //output image
  int nx,       //number of columns of the image
  int ny,       //number of rows of the image
  int nz        //number of channels of the image
)
{
  float *ff=new float[nx*ny*nz];
  for(int i=0;i<nx*ny*nz;i++) ff[i]=(float)f[i];
  iio_save_image_float_vec(fname, ff, nx, ny, nz);
  delete []ff;
}

void save_normalize_image
(
  char *fname,  //file name
  double *f,    //output image
  int nx,       //number of columns of the image
  int ny        //number of rows of the image
)
{
  float max=0, min=999;
  for(int i=0;i<nx*ny;i++)
  {
    if(f[i]<min)min=f[i];
    if(f[i]>max)max=f[i];
  }
  
  float *ff=new float[nx*ny];
  for(int i=0;i<nx*ny;i++) ff[i]=255.0;
  if(max!=min)
    for(int i=0;i<nx*ny;i++) ff[i]=(float)255.0*(f[i]-min)/(max-min);
  iio_save_image_float(fname, ff, nx, ny);
  delete []ff;
}


void save_image
(
  char *fname,  //file name
  float *f,     //output image
  int nx,       //number of columns of the image
  int ny,       //number of rows of the image
  int nz        //number of channels of the image
)
{
  iio_save_image_float_vec(fname, f, nx, ny, nz);
}

/**
 *
 *  Function to save flow fields using the iio library
 *
 */
void save_flow
(
  char *file, //file name
  double *u,  //x component of the optical flow
  double *v,  //y component of the optical flow
  int nx,     //number of columns
  int ny      //number of rows
) 
{
  //save the flow 
  float *f=new float[nx*ny*2];
  for (int i=0; i<nx*ny; i++)
    {
      f[2*i]=u[i];
      f[2*i+1]=v[i];
    }
  iio_save_image_float_vec (file, f, nx, ny, 2);
  delete []f;
}

/**
 *
 *  Function to read the parameters in ascii format
 *  It reads a header with: nparams nx ny
 *  Then it reads the parameters for each pixel
 *
 */
void read
(
  char *file,   //input file name
  double **p,   //parameters to be read
  int &nparams, //number of parameters
  int &nx,      //number of columns 
  int &ny       //number of rows 
)
{
  FILE *fd=fopen(file,"r");
  if(fd!=NULL)
  {
    int result=fscanf(fd,"%d %d %d",&nparams, &nx, &ny);
    if(result==3)
    {
      *p=new double[nx*ny*nparams];
      for(int i=0; i<nx*ny; i++)
      {
        for(int j=0; j<nparams; j++)
          result=fscanf(fd,"%lf", &((*p)[i*nparams+j]));
      }
    }
    fclose(fd);
  }
}


/**
 *
 *  Function to save the parameters in ascii format
 *  It creates a header with: nparams nx ny
 *  Then it stores the parameters for each pixel
 *
 */
void save
(
  char *file,  //output file name 
  double *p,   //parameters to be saved
  int nparams, //number of parameters
  int nx,      //number of columns 
  int ny       //number of rows 
) 
{
  
  FILE *fd=fopen(file,"w");
  fprintf(fd,"%d %d %d\n", nparams, nx, ny);
  for(int i=0; i<nx*ny; i++)
  {
    for(int j=0; j<nparams; j++)
      fprintf(fd,"%f ", p[i*nparams+j]);
    fprintf(fd,"\n");
  }
  fclose(fd);
}


/**
 *
 *  Function to read the parameters in ascii format
 *  It reads a header with: nparams nx ny
 *  Then it reads the parameters 
 *
 */
void read
(
  const char *file, //input file name
  double **p,       //parameters to be read
  int &nparams      //number of parameters
)
{
  FILE *fd=fopen(file,"r");
  *p=NULL;
  if(fd!=NULL)
  {
    int result=fscanf(fd,"%d",&nparams);
    if(result==1)
    {
      *p=new double[nparams];
      for(int j=0; j<nparams; j++)
        result=fscanf(fd,"%lf", &((*p)[j]));
    }
    fclose(fd);
  }
}

/**
 *
 *  Function to save the parameters in ascii format
 *  It creates a header with: nparams nx ny
 *  Then it stores the parameters 
 *
 */
void save
(
  const char *file, //output file name 
  double *p,        //parameters to be saved
  int nparams       //number of parameters
) 
{
  FILE *fd=fopen(file,"w");
  fprintf(fd,"%d\n", nparams);
  for(int j=0; j<nparams; j++)
    fprintf(fd,"%f ", p[j]);
  fprintf(fd,"\n");
  fclose(fd);
}
