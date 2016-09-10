/**@file 

    A* path finding algorithm, for binary images.

	It finds the path between two points by gradually expanding
	shells of accessible points round the two points until they meet.

	Many thanks to Richard Heathfield for the heap (priority queue).

*/
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "heap.h"

#define FILL 0x01
#define FILLMASK 0x01

#define ASET 0x02
#define BSET 0x04
#define SETMASK 0x06

#define NONE 0x00
#define NORTHWEST 0x10
#define NORTH 0x20
#define NORTHEAST 0x30
#define WEST 0x40
#define EAST 0x50
#define SOUTHWEST 0x60
#define SOUTH 0x70
#define SOUTHEAST 0x80
#define LINKMASK 0xF0


typedef struct
{
	int x;
	int y;
	double heuristic;
	double fscore;
	int origin;

} APOINT;

static int heapcompf(const void *Left, int LeftTag, const void *Right, int RightTag);
static double diagonaldistance(int ax, int ay, int bx, int by);
static int traceback(unsigned char *img, int width, int height, int x, int y, int **pathx, int **pathy);
static void reverse(int *x, int N);
static void get3x3(unsigned char *out, unsigned char *img, int width, int height, int x, int y, unsigned char border);

/**
A star path finding algorithm.

@param[in] binary - the binary image
@param width - image width
@param height - image height
@param sx - start point x-coordinate
@param sy - start point y-coordinate
@param ex - end point x coordinate
@param ey - end point y coordinate
@param[out] pathx - return for x-coordinates of path (malloced)
@param[out] pathy - return for y-coordinates of path (malloced)
@returns Number of path points, -1 on fail.

*/
int astar(unsigned char *binary, int width, int height, int sx, int sy, int ex, int ey, int **pathx, int **pathy)
{
	unsigned char *img = 0;
	HEAP *heap = 0;
	int ok;
	APOINT a, b, ap, np;
	int set, otherset;
	unsigned char neighbours[9];
	int i, ii;
	int j;
	int nx, ny;
	int targetx, targety;
	int Npa, Npb;
	int *pathax, *pathay, *pathbx, *pathby;
	int *tpathx, *tpathy;
	int answer = 0;
	
	img = malloc(width * height);
	if (!img)
		goto out_of_memory;
	for (i = 0; i < width*height; i++)
		img[i] = binary[i] ? FILL : 0;

	img[sy*width + sx] |= ASET;
	img[ey*width + ex] |= BSET;

	heap = HeapCreate(100);
	if (!heap)
		goto out_of_memory;

	a.x = sx;
	a.y = sy;
	a.fscore = 0;
	a.heuristic = diagonaldistance(sx, sy, ex, ey);
	ok = HeapInsert(heap, 0, sizeof(APOINT), &a, heapcompf);
	if (ok == 0)
		goto out_of_memory;

	b.x = ex;
	b.y = ey;
	b.fscore = 0;
	b.heuristic = diagonaldistance(sx, sy, ex, sy);
	ok = HeapInsert(heap, 0, sizeof(APOINT), &b, heapcompf);
	if (ok == 0)
		goto out_of_memory;

	while (HeapGetSize(heap) > 0)
	{	
		HeapDelete(heap, 0, 0, &ap, heapcompf);
		get3x3(neighbours, img, width,height, ap.x, ap.y, 0x0);
		set = neighbours[4] & SETMASK;
		if (set == ASET)
		{
			otherset = BSET;
			targetx = ex;
			targety = ey;
		}
		else
		{
			otherset = ASET;
			targetx = sx;
			targety = sy;
		}

		for (j = 0; j < 9; j++)
		{
			nx = ap.x + (j % 3) - 1;
			ny = ap.y + (j / 3) - 1;


			if ((neighbours[j] & FILLMASK) && (neighbours[j] & SETMASK) == 0)
			{
				double cost = 0.0;
				switch (j)
				{
				case 0: cost = 1.414;  img[ny*width + nx] |= (set | SOUTHEAST); break;
				case 1: cost = 1.0;    img[ny*width + nx] |= (set | SOUTH); break;
				case 2: cost = 1.414;  img[ny*width + nx] |= (set | SOUTHWEST); break;
				case 3: cost = 1.0;    img[ny*width + nx] |= (set | EAST); break;
				case 4: break;
				case 5: cost = 1.0;    img[ny*width + nx] |= (set | WEST); break;
				case 6: cost = 1.141;  img[ny*width + nx] |= (set | NORTHEAST); break;
				case 7: cost = 1.0;    img[ny*width + nx] |= (set | NORTH); break;
				case 8: cost = 1.414;  img[ny*width + nx] |= (set | NORTHWEST); break;
				}
				if (j != 4)
				{
					np.x = nx;
					np.y = ny;
					np.heuristic = diagonaldistance(np.x, np.y, targetx, targety);
					np.fscore = ap.fscore + cost;
					ok = HeapInsert(heap, 0, sizeof(APOINT), &np, heapcompf);
					if (ok == 0)
						goto out_of_memory;
				}
			}
			else if ((neighbours[j] & SETMASK) == otherset)
			{
				Npa = traceback(img, width, height, ap.x, ap.y, &pathax, &pathay);
				Npb = traceback(img, width, height, nx, ny, &pathbx, &pathby);
				if (Npa == -1 || Npb == -1)
					goto out_of_memory;
				reverse(pathax, Npa);
				reverse(pathay, Npa);
				tpathx = malloc((Npa + Npb) * sizeof(int));
				tpathy = malloc((Npa + Npb) * sizeof(int));
				if (!tpathx || !tpathy)
					goto out_of_memory;
				for (ii = 0; ii < Npa; ii++)
				{
					tpathx[ii] = pathax[ii];
					tpathy[ii] = pathay[ii];
				}
				for (ii = 0; ii < Npb; ii++)
				{
					tpathx[ii + Npa] = pathbx[ii];
					tpathy[ii + Npa] = pathby[ii];
				}

				if (tpathx[0] != sx || tpathy[0] != sy)
				{
					reverse(tpathx, Npa + Npb);
					reverse(tpathy, Npa + Npb);
				}
				free(pathax);
				free(pathay);
				free(pathbx);
				free(pathby);
				*pathx = tpathx;
				*pathy = tpathy;
				answer = Npa + Npb;
				goto done;
			}
		}
	}
done:
	HeapDestroy(heap);
	free(img);
   return answer;


out_of_memory:
   HeapDestroy(heap);
   free(img);
	return -1;
}

static int heapcompf(const void *Left,
	int LeftTag,
	const void *Right,
	int RightTag)
{
	const APOINT *ap1 = Left;
	const APOINT *ap2 = Right;
	double s1, s2;

	s1 = ap1->heuristic + ap1->fscore;
	s2 = ap2->heuristic + ap2->fscore;

	if (s1 < s2)
		return -1;
	else if (s1 == s2)
		return 0;
	else
		return 1;
}


static int traceback(unsigned char *img, int width, int height, int x, int y, int **pathx, int **pathy)
{
	int N = 0;
	int sx = x;
	int sy = y;
	int *px;
	int *py;
	int i = 0;

	while (img[y*width + x] & LINKMASK)
	{
		switch (img[y*width + x] & LINKMASK)
		{
		case NORTHWEST: x--; y--; break;
		case NORTH: y--; break;
		case NORTHEAST: x++; y--; break;
		case WEST: x--; break;
		case EAST: x++; break;
		case SOUTHWEST: x--;  y++; break;
		case SOUTH: y++; break;
		case SOUTHEAST: x++; y++; break;
		}
		N++;
	}
	N++;
	px = malloc(N * sizeof(int));
	py = malloc(N * sizeof(int));

	x = sx;
	y = sy;

	while (img[y*width + x] & LINKMASK)
	{
		px[i] = x;
		py[i] = y;
		switch (img[y*width + x] & LINKMASK)
		{
		case NORTHWEST: x--; y--; break;
		case NORTH: y--; break;
		case NORTHEAST: x++; y--; break;
		case WEST: x--; break;
		case EAST: x++; break;
		case SOUTHWEST: x--;  y++; break;
		case SOUTH: y++; break;
		case SOUTHEAST: x++; y++; break;
		}
		i++;
	} 
	px[i] = x;
	py[i] = y;

	*pathx = px;
	*pathy = py;
	return N;
out_of_memory:
	free(px);
	free(py);
	*pathx = 0;
	*pathy = 0;
	return -1;
}

static void reverse(int *x, int N)
{
	int i;
	int temp;

	for (i = 0; i < N / 2; i++)
	{
		temp = x[i];
		x[i] = x[N - i - 1];
		x[N - i - 1] = temp;
	}
}

static double diagonaldistance(int ax, int ay, int bx, int by)
{
	int dx, dy;

	dx = abs(ax - bx);
	dy = abs(ay - by);

	if (dx >= dy)
	{
		return (dx - dy) + dy * 1.414;
	}
	else
	{
		return (dy - dx) + dx *1.414;
	}

}


static void get3x3(unsigned char *out, unsigned char *img, int width, int height, int x, int y, unsigned char border)
{
	out[0] = (x > 0 && y > 0) ? img[(y-1)*width + x-1] : border;
	out[1] = (y > 0) ? img[(y-1)*width + x] : border;
	out[2] = (x < width - 1 && y > 0) ? img[(y-1)*width + x+1] : border;
	out[3] = (x > 0) ? img[y*width + x-1] : border;
	out[4] = img[y*width + x];
	out[5] = (x < width - 1) ? img[y*width + x+1] : border;
	out[6] = (x > 0 && y < height - 1) ? img[(y+1)*width + x-1] : border;
	out[7] = (y < height - 1) ? img[(y+1)*width + x] : border;
	out[8] = (x < width - 1 && y < height - 1) ? img[(y+1)*width + x+1] : border;
}


