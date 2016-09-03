/**@file

   Medial axis transform and thin function.

   Used for skeltonisation of images.

*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

static float *euclideandistancetransform(unsigned char *binary, int width, int height);
static int *edt_saito(unsigned char *binary, int width, int height);

void thin(unsigned char *binary, int width, int height);
static int thin_pass(unsigned char *binary, int width, int height, unsigned char *out, int pass);
static void get3x3(unsigned char *out, unsigned char *img, int width, int height, int x, int y, unsigned char border);

/**
  Medial axis transform, retainign distance to perimater

  @param[in] binary - the binary image.
  @param     width - image width
  @param     height - image height
  @returns Transformed iamge as floats (malloced). 
*/
float *medialaxistransformf(unsigned char *binary, int width, int height)
{
	float *answer = 0;
	unsigned char *thinned = 0;
	unsigned char *bordered = 0;
	int *dt = 0;
	int x, y;

	answer = malloc(width *height *sizeof(float));
	bordered = malloc((width + 2) * (height + 2));
	if (!bordered || !answer)
		goto error_exit;

	for (y = 0; y<height; y++)
		for (x = 0; x<width; x++)
			bordered[(y + 1)* (width + 2) + x + 1] = binary[y*width + x];

	for (x = 0; x<width + 2; x++)
	{
		bordered[x] = 0;
		bordered[(height + 1)*(width + 2) + x] = 0;
	}
	for (y = 0; y<height + 2; y++)
	{
		bordered[y*(width + 2)] = 0;
		bordered[y*(width + 2) + width + 1] = 0;
	}

	dt = edt_saito(bordered, width + 2, height + 2);
	if (!dt)
		goto error_exit;

	for (y = 1; y < height + 1; y++)
		for (x = 1; x < width + 1; x++)
		{
		if (dt[y*(width + 2) + x])
		{
			if (dt[(y - 1)*(width + 2) + x - 1] < dt[y*(width + 2) + x] &&
				dt[(y + 1)*(width + 2) + x + 1] <= dt[(y*(width + 2) + x)])
				bordered[y*(width + 2) + x] = 1;
			else if (dt[(y - 1)*(width + 2) + x + 1] <= dt[y*(width + 2) + x] &&
				dt[(y + 1)*(width + 2) + x - 1] < dt[(y*(width + 2) + x)])
				bordered[y*(width + 2) + x] = 1;
			else if (dt[(y - 1)*(width + 2) + x] < dt[y*(width + 2) + x] &&
				dt[(y + 1)*(width + 2) + x] <= dt[(y*(width + 2) + x)])
				bordered[y*(width + 2) + x] = 1;
			else if (dt[y*(width + 2) + x - 1] < dt[y*(width + 2) + x] &&
				dt[y*(width + 2) + x + 1] <= dt[(y*(width + 2) + x)])
				bordered[y*(width + 2) + x] = 1;
			else
				bordered[y*(width + 2) + x] = 0;
		}
		else
			bordered[y*(width + 2) + x] = 0;

		}
	thin(bordered, width + 2, height + 2);
	for (y = 0; y < height; y++)
		for (x = 0; x < width; x++)
			if (bordered[y*(width + 2) + x])
				answer[y*width + x] = sqrt(dt[y*width + x]);
			else
				answer[y*width + x] = 0;

	free(dt);
	free(thinned);
	free(bordered);
	return answer;
error_exit:
	free(dt);
	free(thinned);
	free(bordered);
	free(answer);
	return 0;
}

/**
Binary raster-based medial axis transform.
@param[in] binary - the binary image
@param width - image width
@param height - image height
@returns The medial axis transform.
*/
unsigned char *medialaxistransform(unsigned char *binary, int width, int height)
{
	unsigned char *answer =0;
	unsigned char *thinned = 0;
	unsigned char *bordered = 0;
	int *dt = 0;
	int x, y;

	answer = malloc(width *height);
	bordered = malloc((width + 2) * (height + 2));
	if (!bordered || !answer)
		goto error_exit;

	for (y = 0; y<height; y++)
		for (x = 0; x<width; x++)
			bordered[(y + 1)* (width + 2) + x + 1] = binary[y*width + x];

	for (x = 0; x<width + 2; x++)
	{
		bordered[x] = 0;
		bordered[(height + 1)*(width + 2) + x] = 0;
	}
	for (y = 0; y<height + 2; y++)
	{
		bordered[y*(width + 2)] = 0;
		bordered[y*(width + 2) + width + 1] = 0;
	}

	dt = edt_saito(bordered, width + 2, height + 2);
	if (!dt)
		goto error_exit;

	
	for (y = 1; y < height + 1; y++)
		for (x = 1; x < width +1; x++)
		{
		if (dt[y*(width + 2) + x])
		{
			if (dt[(y - 1)*(width + 2) + x - 1] < dt[y*(width + 2) + x] &&
				dt[(y + 1)*(width + 2) + x + 1] <= dt[(y*(width + 2) + x)])
				bordered[y*(width + 2) + x] = 1;
			else if (dt[(y - 1)*(width + 2) + x + 1] <= dt[y*(width + 2) + x] &&
				dt[(y + 1)*(width + 2) + x - 1] < dt[(y*(width + 2) + x)])
				bordered[y*(width + 2) + x] = 1;
			else if (dt[(y - 1)*(width + 2) + x] < dt[y*(width + 2) + x] &&
				dt[(y + 1)*(width + 2) + x] <= dt[(y*(width + 2) + x)])
				bordered[y*(width + 2) + x] = 1;
			else if (dt[y*(width + 2) + x - 1] < dt[y*(width + 2) + x] &&
				dt[y*(width + 2) + x + 1] <= dt[(y*(width + 2) + x)])
				bordered[y*(width + 2) + x] = 1;
			else
				bordered[y*(width + 2)+x] = 0;
		}
		else
			bordered[y*(width + 2) + x] = 0;
		
		}
	thin(bordered, width + 2, height + 2);
	for (y = 0; y < height; y++)
		for (x = 0; x < width; x++)
			answer[y*width + x] = bordered[(y+1)*(width+2)+x+1];

	free(dt);
	free(thinned);
	free(bordered);
	return answer;
error_exit:
	free(dt);
	free(thinned);
	free(bordered);
	free(answer);
	return 0;
}

/*
gift-wraps edt_saito.
The function doesn't handle the case where set pixels touch the border
Also, it returns the square of the distnace as an integer
*/
static float *euclideandistancetransform(unsigned char *binary, int width, int height)
{
	unsigned char *bordered = 0;
	int *dt = 0;
	float *answer = 0;
	int x, y;

	bordered = malloc((width + 2) * (height + 2));
	if (!bordered)
		goto error_exit;

	for (y = 0; y<height; y++)
		for (x = 0; x<width; x++)
			bordered[(y + 1)* (width + 2) + x + 1] = binary[y*width + x];

	for (x = 0; x<width + 2; x++)
	{
		bordered[x] = 0;
		bordered[(height + 1)*(width + 2) + x] = 0;
	}
	for (y = 0; y<height + 2; y++)
	{
		bordered[y*(width + 2)] = 0;
		bordered[y*(width + 2) + width + 1] = 0;
	}

	dt = edt_saito(bordered, width + 2, height + 2);
	if (!dt)
		goto error_exit;

	answer = malloc(width * height * sizeof(float));
	if (!answer)
		goto error_exit;

	for (y = 0; y<height; y++)
		for (x = 0; x<width; x++)
			answer[y*width + x] = (float)sqrt(dt[(y + 1)*(width + 2) + x + 1]);

	free(dt);
	free(bordered);
	return answer;
error_exit:
	free(bordered);
	free(dt);
	free(answer);
	return 0;
}

/*
* Final implementation by R. Fabbri,
* based on two independent implementations by O. Cuisenaire
* and J. C. Torelli.
*
* PAPER
*    T. Saito and J.I. Toriwaki, "New algorithms for Euclidean distance
*    transformations of an n-dimensional digitised picture with applications",
*    Pattern Recognition, 27(11), pp. 1551-1565, 1994
*
* A nice overview of Saito's method may be found at:
*    Chapter 2 of "Distance transformations: fast algorithms and applications
*    to medical image processing", Olivier Cuisenaire's Ph.D. Thesis, October
*    1999, Université catholique de Louvain, Belgium.
*
*
*/
static int *edt_saito(unsigned char *binary, int width, int height)
{
	int i, j, l, a, b, m, buffer, nsqr, diag1,
		*ptstart, *ptend, *sq, *df2, *buff, *pt, *npt;

	int *map = 0;
	sq = 0;
	buff = 0;


	map = malloc(width * height * sizeof(int));
	if (!map)
		goto error_exit;
	for (i = 0; i<width*height; i++)
		map[i] = binary[i];


	/* this is >= the diagonal minus 1 */
	diag1 = (int)ceil(sqrt((width - 1)*(width - 1) + (height - 1)*(height - 1))) - 1;

	/* Cuisenaire's idea: a LUT with precomputed i*i */
	nsqr = 2 * (diag1 + 1);   /* was: 2*r + 2 in Cuisenaire's code */
	sq = malloc(nsqr * sizeof(int));
	if (!sq)
		goto error_exit;
	for (i = 0; i<nsqr; i++)
		sq[i] = i*i;

	/* buff stores the current column in step 2 */
	buff = malloc(height * sizeof(int));
	if (!buff)
		goto error_exit;

	/*-- Step 1 --*/
	for (j = 0; j<height; j++)
	{
		ptstart = map + j*width;
		ptend = ptstart + width;

		/* forward scan */
		df2 = sq + diag1;  /* the paper: df2 = sq + r, not large enough */
		for (pt = ptstart; pt < ptend; pt++)
			if (*pt)
				*pt = *(++df2);
			else
				df2 = sq;

		/* backward scan */
		df2 = sq + diag1;  /* the paper: df2 = sq + r, not large enough */
		for (--pt; pt != ptstart - 1; --pt)
			if (*pt)
			{
			if (*pt > *(++df2))
				*pt = *df2;
			}
			else
				df2 = sq;
	}


	/*-- Step 2 --*/

	for (i = 0; i<width; i++)
	{
		pt = map + i;

		for (j = 0; j<height; j++, pt += width)
			buff[j] = *pt;

		pt = map + i + width;
		a = 0;
		buffer = buff[0];
		for (j = 1; j < height; j++, pt += width)
		{
			if (a != 0)
				--a;
			if (buff[j] > buffer + 1)
			{
				b = (buff[j] - buffer - 1) / 2;
				if (j + b + 1 > height)
					b = height - 1 - j;

				npt = pt + a*width;
				for (l = a; l <= b; l++)
				{
					m = buffer + sq[l + 1];
					if (buff[j + l] <= m)
						break;   /* go to next column j */
					if (m < *npt)
						*npt = m;
					npt += width;
				}
				a = b;
			}
			else
				a = 0;
			buffer = buff[j];
		}


		a = 0;
		pt -= 2 * width;
		buffer = buff[height - 1];

		for (j = height - 2; j != -1; j--, pt -= width)
		{
			if (a != 0)
				--a;
			if (buff[j] > buffer + 1)
			{
				b = (buff[j] - buffer - 1) / 2;
				if (j < b)
					b = j;

				npt = pt - a*width;
				for (l = a; l <= b; ++l)
				{
					m = buffer + sq[l + 1];
					if (buff[j - l] <= m)
						break;   /* go to next column j */
					if (m < *npt)
						*npt = m;
					npt -= width;
				}
				a = b;
			}
			else
				a = 0;
			buffer = buff[j];
		}

	}

	free(sq);
	free(buff);

	return map;

error_exit:
	free(buff);
	free(sq);
	free(map);
	return 0;
}


/*
* C code from the article
* "Efficient Binary Image Thinning using Neighborhood Maps"
* by Joseph M. Cychosz, 3ksnn64@ecn.purdue.edu
* in "Graphics Gems IV", Academic Press, 1994
*/



/* ---- ThinImage - Thin binary image. -------------------------------- */
/*									*/
/*	Description:							*/
/*	    Thins the supplied binary image using Rosenfeld's parallel	*/
/*	    thinning algorithm.						*/
/*									*/
/*	On Entry:							*/
/*	    image = Image to thin.					*/
/*									*/
/* -------------------------------------------------------------------- */


/* Direction masks:			*/
/*   N	   S	 W     E		*/
static	int	masks[] = { 0200, 0002, 0040, 0010 };

/*	True if pixel neighbor map indicates the pixel is 8-simple and	*/
/*	not an end point and thus can be deleted.  The neighborhood	*/
/*	map is defined as an integer of bits abcdefghi with a non-zero	*/
/*	bit representing a non-zero pixel.  The bit assignment for the	*/
/*	neighborhood is:						*/
/*									*/
/*				a b c					*/
/*				d e f					*/
/*				g h i					*/

static	unsigned char	delete[512] = {
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 1, 0, 0, 1, 1, 0, 1, 1, 1, 0, 0, 1, 1,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 1, 1, 1, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 1, 1,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 1,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 1, 1,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	1, 0, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	1, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 1, 1,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	1, 0, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	1, 0, 1, 1, 1, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 1,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	1, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 1, 1,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	1, 0, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	1, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 1, 1,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	1, 0, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 };

/**
  Thin a binary image using Rosenfeld's parallel thinning 
    algorithm.

  @param[in,out] binary - the bianry image
  @param xsize - image width
  @param ysize - image height
  @note Thinning is a different operation to medial axis
    transform, but similarly skeletonises the binary image.

*/
void	thin(unsigned char *image, int xsize, int ysize)
{
	int		x, y;		/* Pixel location		*/
	int		i;		/* Pass index			*/
	int		pc = 0;	/* Pass count			*/
	int		count = 1;	/* Deleted pixel count		*/
	int		p, q;		/* Neighborhood maps of adjacent*/
	/* cells			*/
	unsigned char		*qb;		/* Neighborhood maps of previous*/
	/* scanline			*/
	int		m;		/* Deletion direction mask	*/
	int width = xsize;

	qb = malloc(xsize);
	qb[xsize - 1] = 0;		/* Used for lower-right pixel	*/

	while (count) {		/* Scan image while deletions	*/
		pc++;
		count = 0;

		for (i = 0; i < 4; i++)
		{

			m = masks[i];

			/* Build initial previous scan buffer.			*/

			p = image[0] != 0;
			for (x = 0; x < xsize - 1; x++)
				qb[x] = p = ((p << 1) & 0006) | (image[x + 1] != 0);

			/* Scan image for pixel deletion candidates.		*/

			for (y = 0; y < ysize - 1; y++) {

				q = qb[0];
				p = ((q << 3) & 0110) | (image[(y + 1)*width] != 0);

				for (x = 0; x < xsize - 1; x++) {
					q = qb[x];
					p = ((p << 1) & 0666) | ((q << 3) & 0110) |
						(image[(y + 1)*width + x + 1] != 0);
					qb[x] = p;
					if (((p&m) == 0) && delete[p]) {
						count++;
						image[y*width + x] = 0;
					}
				}

				/* Process right edge pixel.			*/

				p = (p << 1) & 0666;
				if ((p&m) == 0 && delete[p]) {
					count++;
					image[y*width + xsize - 1] = 0;
				}
			}

			/* Process bottom scan line.				*/

			for (x = 0; x < xsize; x++) {
				q = qb[x];
				p = ((p << 1) & 0666) | ((q << 3) & 0110);
				if ((p&m) == 0 && delete[p]) {
					count++;
					image[(ysize - 1)*width + x] = 0;
				}
			}
		}
		if (pc > xsize && pc > ysize)
			break;
	}

	free(qb);
}

/*
   old version, wasn't efficient
*/
static unsigned char *oldthin(unsigned char *binary, int width, int height)
{
	unsigned char *buff1, *buff2;
	unsigned char *temp;
	int p1, p2;

	buff1 = malloc(width*height);
	buff2 = malloc(width*height);
	if (!buff1 || !buff2)
		goto out_of_memory;
	memcpy(buff1, binary, width *height);
	do
	{
		p1 = thin_pass(buff1, width, height, buff2, 1);
		p2 = thin_pass(buff2, width, height, buff1, 2);
	} while (p1 || p2);

	free(buff2);
	return buff1;
out_of_memory:
	free(buff1);
	free(buff2);
	return 0;
}

static int thin_pass(unsigned char *binary, int width, int height, unsigned char *out, int pass)
{
	int x, y;
	int answer = 0;
	unsigned char pix;
	unsigned char neighbours[9];
	unsigned char round[9];
	int ntor[9] = { 1, 2, 5, 8, 7, 6, 3, 0, 1 };
	int Nneighbours;
	int Ncrossings;
	int i;

	for (y = 0; y < height; y++)
		for (x = 0; x < width; x++)
		{
		pix = 0;
		if (binary[y*width + x])
		{
			pix = 1;
			Nneighbours = 0;
			Ncrossings = 0;
			get3x3(neighbours, binary, width, height, x, y, 0);
			for (i = 0; i < 9; i++)
				round[i] = neighbours[ntor[i]];
			for (i = 0; i < 8; i++)
				Nneighbours += round[i];
			if (Nneighbours >= 2 && Nneighbours <= 6)
			{
				for (i = 0; i < 8; i++)
					if (round[i] == 0 && round[i + 1] == 1)
						Ncrossings++;
				if (Ncrossings == 1)
				{
					if (pass == 1 &&
						round[1] * round[3] * round[5] == 0 &&
						round[3] * round[5] * round[7] == 0)
					{
						pix = 0;
						answer = 1;
					}
					else if (pass == 2 &&
						round[1] * round[3] * round[7] == 0 &&
						round[1] * round[5] * round[7] == 0)
					{
						pix = 0;
						answer = 1;
					}
				}
			}
		}

		out[y*width + x] = pix;
		}

	return answer;
}

static void get3x3(unsigned char *out, unsigned char *img, int width, int height, int x, int y, unsigned char border)
{
	out[0] = (x > 0 && y > 0) ? img[(y - 1)*width + x - 1] : border;
	out[1] = (y > 0) ? img[(y - 1)*width + x] : border;
	out[2] = (x < width - 1 && y > 0) ? img[(y - 1)*width + x + 1] : border;
	out[3] = (x > 0) ? img[y*width + x - 1] : border;
	out[4] = img[y*width + x];
	out[5] = (x < width - 1) ? img[y*width + x + 1] : border;
	out[6] = (x > 0 && y < height - 1) ? img[(y + 1)*width + x - 1] : border;
	out[7] = (y < height - 1) ? img[(y + 1)*width + x] : border;
	out[8] = (x < width - 1 && y < height - 1) ? img[(y + 1)*width + x + 1] : border;
}


