/**
@file Binary Hausdorff routine
*/
#include <stdlib.h>
#include <string.h>


static int dilate(unsigned char *binary, int width, int height, unsigned char *sel, int swidth, int sheight);

/**
  Binary Hausdorff distance.

  The Hausforff distance is the maximum distance from the pixels in A not
  in B to the nearest pixel in B, and vice versa. So it is a measure of
  image similarity. The half-Hausdorff distance just considers A to B or
  B to A, and the Huasdorff distance is trivally the maximum of the two.


  @param[in] imagea - the first image
  @param[in] imageb - the second image
  @param width - image width
  @param height - image height
  @param[out] halfa - return for half-Hausdorff distance a to b
  @param[out] halfb - return for half-Hausdorff distance b to a
  @returns The Hausdorff distnace between image a and image b.
  @note Diagonal steps = 1, not Euclidean distance.
  @note return -1 if either image empty.
*/
int binaryhausdorff(unsigned char *imagea, unsigned char *imageb, int width, int height, int *halfa, int *halfb)
{
	unsigned char *buff;
	unsigned char se[9] = { 1,1,1,1,1,1,1,1,1 };
	int maxit;
	int halfda = -1;
	int halfdb = -1;
	int answer;
	int i, ii;

	buff = malloc(width * height);
	if (!buff)
		goto out_of_memory;

	maxit = width > height ? width : height;

	memcpy(buff, imagea, width * height);
	for (i = 0; i < maxit; i++)
	{
		
		for (ii = 0; ii < width*height; ii++)
			if (imageb[ii] && !buff[ii])
				break;
		if (ii == width * height)
		{
			halfdb = i;
			break;
		}
		dilate(buff, width, height, se, 3, 3);
	}
	memcpy(buff, imageb, width * height);
	for (i = 0; i < maxit; i++)
	{
		for (ii = 0; ii < width*height; ii++)
			if (imagea[ii] && !buff[ii])
				break;
		if (ii == width * height)
		{
			halfda = i;
			break;
		}
		dilate(buff, width, height, se, 3, 3);
	}

	if (halfda == -1 || halfdb == -1)
		answer = -1;
	else
		answer = halfda > halfdb ? halfda : halfdb;


	if (halfa)
		*halfa = halfda;
	if (halfb)
		*halfb = halfdb;
	free(buff);
	return answer;

out_of_memory:
	if (halfa)
		*halfa = -1;
	if (halfb)
		*halfb = -1;
	free(buff);
	return -1;
}

/*
Dilate operation.

@param[in,out] bianry - the binary image
@param width - image width
@param height - image height
@param sel[in] - the selection element
@param swidth - selection element width
@param sheight - selection element height
@returns 0 on sucess, -1 on error.

*/
static int dilate(unsigned char *binary, int width, int height, unsigned char *sel, int swidth, int sheight)
{
	int x, y, sx, sy, ix, iy;
	unsigned char *answer;
	int i;
	int bit;

	answer = malloc(width * height);
	if (!answer)
		return -1;

	for (i = 0; i<width*height; i++)
		answer[i] = 0;

	for (y = 0; y<height; y++)
		for (x = 0; x<width; x++)
		{
			for (sy = 0; sy<sheight; sy++)
				for (sx = 0; sx < swidth; sx++)
				{
					ix = x + sx - swidth / 2;
					iy = y + sy - sheight / 2;
					if (ix < 0 || ix >= width || iy < 0 || iy >= height)
						bit = 0;
					else
						bit = binary[iy * width + ix];
					if (bit == 1 && sel[sy*swidth + sx] == 1)
					{
						answer[y*width + x] = 1;
					}
				}
		}
	for (i = 0; i<width*height; i++)
		binary[i] = answer[i];
	free(answer);

	return 0;


}
