/**
@file Binary Hausdorff routine
*/
#include <stdlib.h>
#include <string.h>
#include "binaryutils.h"

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
		{
			printf("%d", buff[ii]);
			if ((ii % width) == width - 1)
				printf("\n");
		}
		printf("\n");

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