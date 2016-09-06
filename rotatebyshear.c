/**@file

  Rotate by shear.

  By Malcolm McLean

  Rotate an image using the shearing method, which avoids sampling
   artifacts as preserves pixel values.

  Part of the binary image processing library.
*/
#include <stdlib.h>
#include <math.h>

#include <assert.h>

#define PI 3.1415926535897932384626433832795

/**
  Rotate an image, using the shearing method.

  @param[in] binary - the binary or colour-indexed image
  @param width - image width
  @param height - image height
  @param cx - centre x, y co-ordinates (pass in 1.5, 1.5 for the centre of a 3x3 image)
  @param cy - centre x, y co-ordinates (pass in 1.5, 1.5 for the centre of a 3x3 image)
  @param theta - angle to rotate by
  @param out[out] - buffer for output (can't be the same as input)
  @returns: 0 for success

  @note conventional image rotation by the matrix method causes destination pixels to be
    sub-samples of source pixels. This isn't a problem with continous tone images where
	the new pixel values can be obtained by interpolation. However with binary or 
	colour-index images, interpolation isn't possible. The shearing method preserves the
	pixels, at some cost in rotational accuracy.

	@image html maggie.jpg  Margaret Thatcher (1925-2013) greyscale photograph
	@image html maggierotshear.gif Mrs Thatcher rotated
	@image html test.gif  Test card (256 colour-indexed)
	@image html testrotshear.gif Test card rotated

  */
int rotatebyshear(unsigned char *binary, int width, int height, double cx, double cy, double theta, unsigned char *out)
{
	double alpha;
	double beta;
	int dpx;
	int tx, ty;
	int x, y;

	assert(binary != out);
	theta = fmod(theta, 2 * PI);

	if(theta >= -PI/2 && theta <= PI/2)
	{
	  alpha = -tan(theta/2);
      beta = sin(theta);


      for(y=0;y<height;y++)
	  {
	     dpx = (int) floor(alpha * (y - cy) + 0.5);
	     for(x=0;x<width;x++)
	     {
		   ty = y + (int) floor(beta * (x + dpx - cx) + 0.5);
		   tx = x + dpx + (int) floor(alpha * (ty - cy) + 0.5); 
		   if(tx >= 0 && tx < width && ty >= 0 && ty < height)
		     out[y*width+x] = binary[ty*width+tx];
		   else
		     out[y*width+x] = 0;
	     }
	  }
	}
	else
	{
		alpha = -tan( (theta + PI) / 2);
		beta = sin(theta + PI);
		for(y=0;y<height;y++)
	    {
	     dpx = (int) floor(alpha * (y - cy) + 0.5);
	     for(x=0;x<width;x++)
	     {
		   ty = y + (int) floor(beta * (x + dpx - cx) + 0.5);
		   tx = x + dpx + (int) floor(alpha * (ty - cy) + 0.5);
		   tx = (int) (cx-(tx - cx));
		   ty = (int) (cy-(ty - cy));
		   if(tx >= 0 && tx < width && ty >= 0 && ty < height)
		     out[y*width+x] = binary[ty*width+tx];
		   else
		     out[y*width+x] = 0;
	     }
	  }
	}

	return 0;
}


