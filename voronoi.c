/**@file

  Discrete Voronoi cell code.

  Calculate Voronoi cells for a binary image. Useful if
  you don't need exact Voronoi tesselation.

  by Malcolm McLean
*/
#include <stdlib.h>
#include <math.h>

static int compcells(const void *e1, const void *e2);
static void get3x3(int *out, int *binary, int width, int height, int x, int y, int border);
static int *edt_saito(unsigned char *binary, int width, int height);

typedef struct
{
	int x;
	int y;
	int d2;
} CELL;

/**
  Calculate discrete Voronoi cells.
  
  @param[in]   seeds - set empty cells to -1, filled cells to the value (usually
	   unique for each point)
  @param	 width - image width
  @param	 height - image height
  @returns 0 on success, -1 on fail
  @image html voronoiseeds.gif The seeds
  @image html voronoiregions.gif Voronoi regions for seeds

  On output, seeds will be expanded to fill the Voronoi cell
  Where there is conflict, the top left-most seed wins (change
  neighbour scan routine to adjust).
*/
int discrete_voronoi(int *seeds, int width, int height)
{
	unsigned char *binary = 0;
	int *edt = 0;
	CELL *cells = 0;
	int neighbours[9];
	int i, ii;
	int mind;
	int nx, ny;

	binary = malloc(width  * height);
	if (!binary)
		goto error_exit;

	for (i = 0; i < width*height; i++)
		binary[i] = (seeds[i] == -1) ? 1 : 0;

	edt = edt_saito(binary, width, height);
	if (!edt)
		goto error_exit;
	cells = malloc(width * height * sizeof(CELL));

	int x, y;
	for (y = 0; y < height; y++)
	{
		for (x = 0; x < width; x++)
		{
			cells[y*width + x].x = x;
			cells[y*width + x].y = y;
			cells[y*width + x].d2 = edt[y *width + x];
		}
	}

	qsort(cells, width*height, sizeof(CELL), compcells);

	for (i = 0; i < width*height; i++)
	{
		if (cells[i].d2 == 0)
			continue;
		get3x3(neighbours, seeds, width, height, cells[i].x, cells[i].y, -1);
		mind = width * height * 2;
		for (ii = 1; ii < 9; ii+=2)
		{
			if (neighbours[ii] != -1)
			{
				nx = cells[i].x + (ii % 3) - 1;
				ny = cells[i].y + (ii / 3) - 1;
				if ( mind > edt[ny * width + nx])
				{
					mind = edt[ny*width + nx];
					seeds[cells[i].y * width + cells[i].x] = neighbours[ii];
				}
			}
		}
	}

	free(cells);
	free(edt);
	free(binary);

	return 0;
error_exit:
	free(cells);
	free(edt);
	free(binary);
	return -1;

}

/* sort cells by distance*/
static int compcells(const void *e1, const void *e2)
{
	const CELL *c1 = e1;
	const CELL *c2 = e2;

	return c1->d2 - c2->d2;
}

/*
get 3x3 neighbourhood, padding for boundaries
Params: out - return pointer for neighbourhood
binary - the binary image
width - image width
height - image height
x, y - centre pixel x, y co-ordinates
border - value to pad borders with.
Notes: pattern returned is
0  1  2
3  4  5
6  7  8
where 4 is the pixel at x, y.

*/
static void get3x3(int *out, int *binary, int width, int height, int x, int y, int border)
{
	if (y > 0 && x > 0)        out[0] = binary[(y - 1)*width + x - 1]; else out[0] = border;
	if (y > 0)                 out[1] = binary[(y - 1)*width + x];   else out[1] = border;
	if (y > 0 && x < width - 1)  out[2] = binary[(y - 1)*width + x + 1]; else out[2] = border;
	if (x > 0)                 out[3] = binary[y*width + x - 1];     else out[3] = border;
	out[4] = binary[y*width + x];
	if (x < width - 1)           out[5] = binary[y*width + x + 1];     else out[5] = border;
	if (y < height - 1 && x > 0) out[6] = binary[(y + 1)*width + x - 1]; else out[6] = border;
	if (y < height - 1)          out[7] = binary[(y + 1)*width + x];   else out[7] = border;
	if (y < height - 1 && x < width - 1) out[8] = binary[(y + 1)*width + x + 1]; else out[8] = border;
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