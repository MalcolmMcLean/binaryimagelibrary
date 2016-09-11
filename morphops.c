#include <stdlib.h>
#include <math.h>
#include <assert.h>

/**
   Create a diamond-like structuring element

   @param radius - diamond radius 9distance from centre to tip)
   @param[out] swidth - return for struturing element width
   @param[out] sheight - return for structuring element height
   @returns The structuring element, 0 on fail.
*/
unsigned char *sediamond(int radius, int *swidth, int *sheight)
{
	unsigned char *answer;
	int w;
	int x, y, x0, y0;

	w = (radius * 2) - 1;
	answer = malloc(w * w);
	if (!answer)
		goto out_of_memory;

	x0 = w / 2;
	y0 = w / 2;

	for (y = 0; y < w; y++)
	{
		for (x = 0; x < w; x++)
		{
			if (abs(x - x0) + abs(y - y0) < radius)
				answer[y*w + x] = 1;
			else
				answer[y*w + x] = 0;
		}
	}
	if (swidth)
		*swidth = w;
	if (sheight)
		*sheight = w;
	return answer;


out_of_memory:
	if (swidth)
		*swidth = -1;
	if (sheight)
		*sheight = -1;
	return 0;
}

/**
   Create a disk-like structuring element
   @param radius - disk radius
   @param[out] swidth - return for structuring element width
   @param[out] sheight - return for structuring element height
   @returns - The structuring element, 0 on fail
*/
unsigned char *sedisk(int radius, int *swidth, int *sheight)
{
	unsigned char *answer;
	int w;
	int x, y, x0, y0;

	w = (radius * 2) - 1;
	answer = malloc(w * w);
	if (!answer)
		goto out_of_memory;

	x0 = w / 2;
	y0 = w / 2;

	for (y = 0; y < w; y++)
	{
		for (x = 0; x < w; x++)
		{
			if ((x - x0)*(x - x0) + (y - y0)*(y - y0) < radius * radius)
				answer[y*w + x] = 1;
			else
				answer[y*w + x] = 0;
		}
	}
	if (swidth)
		*swidth = w;
	if (sheight)
		*sheight = w;
	return answer;


out_of_memory:
	if (swidth)
		*swidth = -1;
	if (sheight)
		*sheight = -1;
	return 0;
}

/**
  Create a structuring element which is an angled line.

  @param length - line length
  @param width - line width
  @param theta - line angle
  @param[out] swidth - return for structuring elelemt width
  @param[out] sheight - return for structuring element height
  @returns The structuring element, 0 on fail.
*/
unsigned char *seline(double length, double width, double theta, int *swidth, int *sheight)
{
	double costheta, sintheta;
	double ox, oy;
	double dx, dy;
	double rx, ry;
	int w, h;
	int x, y;
	unsigned char *answer;

	costheta = cos(theta);
	sintheta = sin(theta);

	dx = length/2 * costheta + width/2 * sintheta;
	dy = length / 2 * -sintheta + width / 2 * costheta;
	w = (int) ceil(fabs(dx));
	h = (int) ceil(fabs(dy));
	w = (w * 2) - 1;
	h = (h * 2) - 1;

	if (w < 1)
		w = 1;
	if (h < 1)
		h = 1;

	answer = malloc(w * h);
	if (!answer)
		goto out_of_memory;

	ox = w / 2 + 0.5;
	oy = h / 2 + 0.5;

	for (y = 0; y < h; y++)
	{
		for (x = 0; x < w; x++)
		{
			dx = (x + 0.5) - ox;
			dy = (y + 0.5) - oy;

			rx = dx *costheta - dy * sintheta;
			ry = dx * sintheta + dy * costheta;

			if (fabs(rx) < length / 2.0 || ry < width / 2)
				answer[y*w + x] = 1;
			else
				answer[y*w + x] = 0;
		}
	}

	if (swidth)
		*swidth = w;
	if (sheight)
		*sheight = h;
	return answer;
out_of_memory:
	
	if (swidth)
		*swidth = -1;
	if (sheight)
		*sheight = -1;
	return 0;
}

/**
   Create a structuring element which is an octagon.

   @param radius - distance from centre to tip, must be multiple of 3 pixels.
   @param swidth - return for structuring element width
   @param sheight - return for structuring element height
   @returns The structuring element, 0 on fail.

*/
unsigned char *seoctagon(int radius, int *swidth, int *sheight)
{
	unsigned char *answer;
	int w;
	int x, y, x0, y0;

	assert((radius % 3) == 0);

	w = (radius * 2) - 1;
	answer = malloc(w * w);
	if (!answer)
		goto out_of_memory;

	x0 = w / 2;
	y0 = w / 2;

	for (y = 0; y < w; y++)
	{
		for (x = 0; x < w; x++)
		{
			if (abs(x - x0) < radius / 3)
				answer[y*w + x] = 1;
			else if (abs(y - y0) < radius / 3)
				answer[y*w + x] = 1;
			else if (abs(x - x0) + abs(y - y0) < radius)
				answer[y*w + x] = 1;
			else
				answer[y*w + x] = 0;
		}
	}
	if (swidth)
		*swidth = w;
	if (sheight)
		*sheight = w;
	return answer;


out_of_memory:
	if (swidth)
		*swidth = -1;
	if (sheight)
		*sheight = -1;
	return 0;


}

/**
  Create a structurng element which sia rectangle

  @param width - rectangle width
  @param height - rectangle height
  @param[out] swidth - return for structuring element width
  @param[out] sheight - return for structuring element height
  @returns The structuring element, 0 on fail.

*/
unsigned char *serectangle(int width, int height, int *swidth, int *sheight)
{
	unsigned char *answer;
	int x, y;
	int w, h;

	if (width % 2)
		w = width;
	else
		w = width + 1;

	if (height % 2)
		h = height;
	else
		h = height + 1;

	answer = malloc(w * h);
	if (!answer)
		goto out_of_memory;
	for (y = 0; y < h; y++)
		for (x = 0; x < w; x++)
		{
			if (x < width && y < height)
				answer[y*w + x] = 1;
			else
				answer[y*w + x] = 0;
		}

	if (swidth)
		*swidth = w;
	if (sheight)
		*sheight = h;

	return answer;

out_of_memory:
	if (swidth)
		*swidth = -1;
	if (sheight)
		*sheight = -1;
	return 0;
}

/**
  Create a structuring element which is a square.

  @param width - the square width and height
  @params swidth - return for structuring element width
  @param  sheight - return fr structuring element height
  @returns The structuring element, 0 on fail.
*/
unsigned char *sesquare(int width, int *swidth, int *sheight)
{
	unsigned char *answer;
	int x, y;
	int w;

	if (width % 2)
		w = width;
	else
		w = width + 1;

	answer = malloc(w * w);
	if (!answer)
		goto out_of_memory;

	for (y = 0; y < w; y++)
		for (x = 0; x < w; x++)
		{
			if (y < width && x < width)
				answer[y*w + x] = 1;
			else
				answer[y*w + x] = 0;
		}
	if (swidth)
		*swidth = w;
	if (sheight)
		*sheight = w;
	return answer;

out_of_memory:
	if (swidth)
		*swidth = -1;
	if (sheight)
		*sheight = -1;
	return 0;
}
