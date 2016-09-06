#include <stdlib.h>
#include <assert.h>
/**@file*
  Halftone functions.

  Half toning is when we represent a greyscale iamge as a binary image,
  using filters and dithering to give a visual representation of the
  grey.

  @image html maggie.jpg  Margaret Thatcher (1925-2013) greyscale photograph

*/

/**
  Halftoning with random dither.
  @param grey - the greyscale image
  @param width - image width
  @param height - image height
  @returns Halftoned binary image.

  @image html maggierandom.gif
*/
unsigned char *randomhalftone(unsigned char *grey, int width, int height)
{
  int x, y;
  int *I = 0;
  unsigned char *answer = 0;
  int err;

  answer = malloc(width * height);
  if(!answer)
	  goto error_exit;

  I = malloc( (width+1) * (height +1) * sizeof(float));
  if(!I)
	  goto error_exit;
  for(y=0;y<height;y++)
    for(x=0;x<width;x++)
	  I[y*(width+1)+x] = grey[y*width+x];

  for(y=0;y<height;y++)
    for(x=0;x<width;x++)
	{
	  if(I[y*(width+1)+x] > 127)
	  {
	    answer[y*width+x] = 1;
		err = I[y*(width+1)+x] - 255;
	  }
	  else
	  {
	    answer[y*width+x] = 0;
        err = I[y*(width+1)+x];
	  }
	  switch(rand() % 3)
	  {
	  case 0: I[y*(width+1)+x+1] += err; break;
	  case 1: I[(y+1)*(width+1)+x] += err; break;
	  case 2: I[(y+1)*(width+1)+x+1] += err; break;
	  }
	}
  free(I);
  return answer;

error_exit:
  free(I);
  free(answer);
  return 0;
}


/**
Halftoning with Floyd-Steinberg error diffusion.
@param grey - the greyscale image
@param width - image width
@param height - image height
@returns Halftoned binary image.
@image html maggiefloyd.gif
*/
unsigned char *floydsteinberg(unsigned char *grey, int width, int height)
{
  float *I = 0;
  float a = 7/16.0f;
  float b = 3/16.0f;
  float c = 5/16.0f;
  float d = 1/16.0f;
  float err;
  unsigned char *answer = 0;
  int x, y;

  I = malloc( (width +1)* (height +1) * sizeof(float));
  if(!I)
    goto error_exit;

  for(y=0;y<height;y++)
    for(x=0;x<width;x++)
      I[y*(width+1)+x] = grey[y*width+x]/255.0f;
 
  answer = malloc(width * height);
  if(!answer)
    goto error_exit;

  for (y = 0; y < height; y++) 
  {
    for (x = 0; x < width; x++) 
	{
      answer[y*width+x]  = (I[y*(width+1)+x] + 0.5) > 1 ? 1 : 0;
      err = I[y*(width+1)+x] - answer[y*width+x];
      I[y*(width+1)+x+1] += a*err;
	  if(x)
		I[(y+1)*(width+1)+x-1] += b*err;
      I[(y+1)*(width+1)+x] += c*err;
      I[(y+1)*(width+1)+x+1] += d*err;
	}
  }

  free(I);
  return answer;
error_exit:
  free(I);
  free(answer);
  return 0;
}

/**
  Halftoning with Stucki error diffusion.
  @param grey - the greyscale image
  @param width - image width
  @param height - image height
  @returns Halftoned binary image.
  @image html maggiestucki.gif
*/
unsigned char *stucki(unsigned char *grey, int width, int height)
{
	float *I = 0;
	unsigned char *answer = 0;
    int i, ii;
	int x, y;
    float err;
	float mtx[3][5] = 
	{
		{0.0f/42, 0.0f/42, 0.0f/42, 8.0f/42, 4.0f/42},
		{2.0f/42, 4.0f/42, 8.0f/42, 4.0f/42, 2.0f/42},
		{1.0f/42, 2.0f/42, 4.0f/42, 2.0f/42, 1.0f/42},
	};

	answer = malloc(width * height);
	if(!answer)
		goto error_exit;

	I = malloc(width * height * sizeof(float));
	if(!I)
	  goto error_exit;

	for(i=0;i<width*height;i++)
      I[i] = grey[i];

	for(y=0;y<height;y++)
      for(x=0;x<width;x++)
	  {
		  answer[y*width+x] = I[y*width+x] > 127 ? 1 : 0;
		  err = I[y*width+x] - (answer[y*width+x] ? 255 : 0);
		  for(i=0;i<3;i++)
		    for(ii=0;ii<5;ii++)
			{
              if(y + i >= height || x + ii-2 < 0 || x + ii-2 >= width)
				  continue;
              I[(y+i)*width+x+ii-2] += err * mtx[i][ii];
			}
	  }

	  free(I);

	  return answer;
error_exit:
	  free(I);
	  free(answer);

	  return 0;
}

/* ==========================================================
Bayer ordered dispersed dot dithering


Function taken from "Ordered Dithering, Stephen Hawley, Graphics Gems, Academic Press, 1990"
This function is used to generate a Bayer dithering matrice whose dimension are 2^size by 2^size
*/
static int dithervalue(int x, int y, int size) 
{
	int d = 0;
	/*
	 * calculate the dither value at a particular
	 * (x, y) over the size of the matrix.
	 */
	while (size-- > 0 )	
	{
		/* Think of d as the density. At every iteration,
		 * d is shifted left one and a new bit is put in the
		 * low bit based on x and y. If x is odd and y is even,
		 * or x is even and y is odd, a bit is put in. This
		 * generates the checkerboard seen in dithering.
		 * This quantity is shifted left again and the low bit of
		 * y is added in.
		 * This whole thing interleaves a checkerboard bit pattern
		 * and y's bits, which is the value you want.
		 */
		d = (((d << 1) | ( (x&1) ^ (y&1) )) << 1) | (y&1) ;
		x >>= 1;
		y >>= 1;
	}
	return d;
}

/**
  Ordered dithering with a Bayer matrix of size 2^order by 2^order.
  @param  grey - the greycale image
  @param  width - image width
  @param  height - image height
  @param order - size of matrix (1-4)
  @returns halftoned binary image
  @image html maggiedisp1.gif order = 1
  @image html maggiedisp2.gif order = 2
  @image html maggiedisp3.gif order = 3
  @image html maggiedisp4.gif order = 4
*/

unsigned char *ordereddisperseddot(unsigned char *grey, int width, int height, int order) 
{
	int x, y, pixel;
	unsigned char *answer = 0;
    unsigned char *matrix = 0;
	int scale;
	int l;
	int i;

	assert(order > 0 && order <= 4);

	answer = malloc(width* height);
	if(!answer)
		goto error_exit;;

	/* build the dithering matrix*/	
	l = (1 << order);
	matrix = malloc(l*l);
	if(!matrix)
		goto error_exit;

	for(i = 0; i < l*l; i++) 
	  matrix[i] = dithervalue(i % l, i / l, order);


	/* perform the dithering */
	scale = 8 - 2*order;
	for(y = 0; y < height; y++) 
	{
		for(x = 0; x < width; x++) 
		{
			pixel = grey[y*width+x] >> scale;	/* scale values to 0..(2^order x 2^order) */
			if(pixel > matrix[(x % l) + l * (y % l)]) 
				answer[y*width+x] = 1;
			else 
				answer[y*width+x] = 0;
		}
	}

	free(matrix);

	return answer;
error_exit:
	free(answer);
	free(matrix);
	return 0;
}

/**
 Ordered clustered dot dithering.

 @param[in] grey - the greyscale image
 @param width - image width
 @param height - image height
 @param order - 3, 4, or8
 @returns The dithered image.

 @image html maggieclust3.gif order = 3
 @image html maggieclust4.gif order = 4
 @image html maggieclust8.gif order = 8

NB : The predefined dither matrices are the same as matrices used in 
the Netpbm package (http://netpbm.sourceforge.net) and are defined in Ulichney's book.
See also : The newsprint web site at http://www.cl.cam.ac.uk/~and1000/newsprint/
for more technical info on this dithering technique
*/
unsigned char *orderedclustereddot(unsigned char *grey, int width, int height, int order) 
{
	/* Order-3 clustered dithering matrix. */
	int cluster3[] = {
	  9,11,10, 8, 6, 7,
	  12,17,16, 5, 0, 1,
	  13,14,15, 4, 3, 2,
	  8, 6, 7, 9,11,10,
	  5, 0, 1,12,17,16,
	  4, 3, 2,13,14,15
	};

	/* Order-4 clustered dithering matrix. */ 
	int cluster4[] = {
	  18,20,19,16,13,11,12,15,
	  27,28,29,22, 4, 3, 2, 9,
	  26,31,30,21, 5, 0, 1,10,
	  23,25,24,17, 8, 6, 7,14,
	  13,11,12,15,18,20,19,16,
	  4, 3, 2, 9,27,28,29,22,
	  5, 0, 1,10,26,31,30,21,
	  8, 6, 7,14,23,25,24,17
	};

	/* Order-8 clustered dithering matrix. */ 
	int cluster8[] = {
	   64, 69, 77, 87, 86, 76, 68, 67, 63, 58, 50, 40, 41, 51, 59, 60,
	   70, 94,100,109,108, 99, 93, 75, 57, 33, 27, 18, 19, 28, 34, 52,
	   78,101,114,116,115,112, 98, 83, 49, 26, 13, 11, 12, 15, 29, 44,
	   88,110,123,124,125,118,107, 85, 39, 17,  4,  3,  2,  9, 20, 42,
	   89,111,122,127,126,117,106, 84, 38, 16,  5,  0,  1, 10, 21, 43,
	   79,102,119,121,120,113, 97, 82, 48, 25,  8,  6,  7, 14, 30, 45,
	   71, 95,103,104,105, 96, 92, 74, 56, 32, 24, 23, 22, 31, 35, 53,
	   65, 72, 80, 90, 91, 81, 73, 66, 62, 55, 47, 37, 36, 46, 54, 61,
	   63, 58, 50, 40, 41, 51, 59, 60, 64, 69, 77, 87, 86, 76, 68, 67,
	   57, 33, 27, 18, 19, 28, 34, 52, 70, 94,100,109,108, 99, 93, 75,
	   49, 26, 13, 11, 12, 15, 29, 44, 78,101,114,116,115,112, 98, 83,
	   39, 17,  4,  3,  2,  9, 20, 42, 88,110,123,124,125,118,107, 85,
	   38, 16,  5,  0,  1, 10, 21, 43, 89,111,122,127,126,117,106, 84,
	   48, 25,  8,  6,  7, 14, 30, 45, 79,102,119,121,120,113, 97, 82,
	   56, 32, 24, 23, 22, 31, 35, 53, 71, 95,103,104,105, 96, 92, 74,
	   62, 55, 47, 37, 36, 46, 54, 61, 65, 72, 80, 90, 91, 81, 73, 66
	};

	int x, y, pixel;
	unsigned char *answer = 0;
	int l, scale;
	int *matrix = 0;

	assert(order == 3 || order == 4 || order == 8);
	
	answer = malloc(width * height);
	if(!answer) 
		return 0;

	/* select the dithering matrix */
	switch(order) {
		case 3:
			matrix = &cluster3[0];
			break;
		case 4:
			matrix = &cluster4[0];
			break;
		case 8:
			matrix = &cluster8[0];
			break;
	}

	/* scale the dithering matrix */
	l = 2 * order;
    scale = 256 / (l * order);
	for(y = 0; y < l; y++) 
		for(x = 0; x < l; x++)
			matrix[y*l + x] *= scale;
		

	/* perform the dithering */
	for(y = 0; y < height; y++) 
	{
		for(x = 0; x < width; x++) 
		{
			pixel = grey[y*width+x];
			if(pixel >= matrix[(y % l) + l * (x % l)]) 
				answer[y*width+x] = 1;
			else 
				answer[y*width+x] = 0;
		}
	}

	return answer;
}


