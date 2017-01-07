#include <math.h>
#include "moments.h"

/**
   Get binary momenents of an image
   
   @param[in] binary - the image
   @param width - image width
   @param height - image height
   @param[out] ret - structure giving moments

   Notes:
     Binary momenets give a translation-independent description
     of the image. The raw momenets are call mu_xy where x and y
     represent the multipication by y.

     We can use the raw momenets to obtain acovariance matrix,
     take eigen values and eighen vectors, and thus rotation and 
     elongation.

 */
void binarymoments(unsigned char *binary, int width, int height, MOMENTS *ret){
  int x, y;

  int M_00  =0;
  int M_10 = 0;
  int M_01 = 0;
  int M_11 = 0;
  int M_20 = 0;
  int M_02 = 0;
  int M_21 = 0;
  int M_12 = 0;
  int M_30 = 0;
  int M_03 = 0;
  double xbar, ybar;
  double lambda, lambda_1, lambda_2;
  double adj;

  for(y=0;y<height;y++)
    for(x=0;x<width;x++)
    {
      if(binary[y*width+x])
      {
          M_00 += 1;
          M_10 += x;
          M_01 += y;
          M_11 += x * y;
          M_20 += x*x;
          M_02 += y*y;
          M_21 += x*x*y;
          M_12 += x*y*y;
          M_30 += x*x*x;
          M_03 += y*y*y;
      }
    }
    xbar = M_10/(double) M_00;
    ybar = M_01/(double) M_00;
   
    ret->mu_00 = M_00;
    ret->mu_11 = M_11 - xbar * M_01;
    ret->mu_20 = M_20 - xbar * M_10;
    ret->mu_02 = M_02 - ybar * M_01;
    ret->mu_21 = M_21 - 2*xbar*M_11 - ybar*M_20 + 2*xbar*xbar*M_01;
    ret->mu_12 = M_12 - 2*ybar*M_11 - xbar*M_02 + 2*ybar*ybar*M_10;
    ret->mu_30 = M_30 - 3*xbar*M_20 +2*xbar*xbar*M_10;
    ret->mu_03 = M_03 - 3*ybar*M_02 +2*ybar*ybar*M_01;

    ret->cov[0][0] = ret->mu_20/ret->mu_00;
    ret->cov[1][1] = ret->mu_02/ret->mu_00;
    ret->cov[0][1] = ret->mu_11/ret->mu_00;
    ret->cov[1][0] = ret->cov[0][1];

    ret->theta = 0.5 * atan(2*ret->cov[0][1] /(ret->cov[0][0] - ret->cov[1][1]));
    lambda = (ret->cov[0][0] + ret->cov[1][1])/2;
    adj = sqrt(4 * ret->cov[1][1]*ret->cov[1][1] + (ret->cov[0][0] - ret->cov[1][1])*(ret->cov[0][0] - ret->cov[1][1]))/2;

    lambda_1 = lambda + adj;
    lambda_2 = lambda - adj;
    ret->eccentricity = sqrt(1.0 - lambda_2/lambda_1);
    ret->xbar = xbar;
    ret->ybar = ybar;
    ret->area = M_00;

    return;
}

#if 0

/* unit test code */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int binarymomentsmain(void)
{
  unsigned char *image;
  int width = 100;
  int height = 60;
  MOMENTS moments;
  int i;
  int x, y;

  image = malloc(width * height);
  memset(image, 0, width * height);

  for(i=10;i<90;i++)
  {
  image[i/2*width+i] = 1;
  image[(i/2+1)*width+i] = 1;
  }

  for(y=0;y<height;y++)
  {
    for(x=0;x<width;x++)
    {
       if( (x-20)*(x-20) + (y-20) *(y-20) < 10*10)
		image[y*width+x] = 1;
    }
  }

  binarymoments(image, width, height, &moments);
   printf("area %f\n", moments.area);
   printf("eccentricity %f\n", moments.eccentricity);
   printf("theta %f (%f)\n", moments.theta, moments.theta/3.14 * 180);
   return 0;
}
#endif
