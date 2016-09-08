/**@file 

    A* path finding algorithm, for binary images.

	It finds the path between two points by gradually expanding
	shells of accessible points round the two points until they meet.

*/
#include <stdlib.h>
#include <string.h>

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
  unsigned char *data;
  int bufflen;
  int N;
  int elsize;
  int(*compfunc)(void *ctx, void *e1, void *e2);
  void *ctx;
} PRIORITY_QUEUE;

PRIORITY_QUEUE *priority_queue(int elsize, int(*compfunc)(void *ctx, void *e1, void *e2), void *ctx)
{
	PRIORITY_QUEUE *q;

	q = malloc(sizeof(PRIORITY_QUEUE));
	q->data = 0;
	q->bufflen = 0;
	q->N = 0;
	q->elsize = elsize;
	q->ctx = ctx;
	return q;
	out_of_memory:
	return 0;
}

void killpriority_queue(PRIORITY_QUEUE *q)
{
	if (q)
	{
		free(q);
		free(q->data);
	}
}

int priority_queue_getsize(PRIORITY_QUEUE *q)
{
	return q->N;
}

int priority_queue_insert(PRIORITY_QUEUE *q, void *item)
{
	void *temp;
	int newbufflen;
	int i;

	if (q->bufflen < q->N + 1)
	{
		newbufflen = q->bufflen + q->bufflen/2 + 10;
		temp = realloc(q->data, newbufflen * q->elsize);
		if (temp)
		{
			q->data = temp;
			q->bufflen = newbufflen;
		}
		else
			goto out_of_memory;
	}
	for (i = 0; i < q->N; i++)
	{
		if ((*q->compfunc)(q->ctx, q->data + q->elsize * i, item))
		{
			break;
		}
	}
	memmove(q->data + (i + 1) * q->elsize, q->data + i * q->elsize, (q->N - i) * q->elsize);
	memcpy(q->data + (i * q->elsize), item, q->elsize);
	q->N++;
	return 0;
out_of_memory:
	return -1;
}

int priority_queue_popfront(PRIORITY_QUEUE *q, void *item)
{
	if (q->N)
	{
		memcpy(item, q->data, q->elsize);
		memmove(q->data, q->data + q->elsize, (q->N - 1) * q->elsize);
		q->N--;
	}
}

typedef struct
{
	int x;
	int y;
	double heuristic;
	double fscore;
	int origin;

} APOINT;

typedef struct
{
	unsigned char *img;  /**< image, set up with direction data to source. */
	int width;           /**< image width. */
	int height;          /**< image height. */
	PRIORITY_QUEUE *q;
	int sx;
	int sy;
	int ex;
	int ey;
	int *pathx;          /**< path x co-ordinates. */
	int *pathy;          /**< path y co-ordinates. */
	int Npath;           /**< size of path. */

} ASTAR;

double diagonaldistance(int ax, int ay, int bx, int by)
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

int astarcomp(void *ctx, void *e1, void *e2)
{
	ASTAR *as = ctx;
	APOINT *ap1 = e1;
	APOINT *ap2 = e2;
	double s1, s2;

	s1 = ap1->heuristic + ap2->fscore;
	s2 = ap2->heuristic + ap2->fscore;

	if (s1 > s2)
		return -1;
	else if (s1 == s2)
		return 0;
	else
		return 1;
}

/**
A star path finding algorithm.

@param[in] binary - the binary image
@param width - image width
@param height - image height
@param sx - start point x-coordiante
@param sy - start point y-coordiante
@param ex - end point x coordiante
@param ey - end point y coordiante
@param[out] pathx - return for x-coordiantes of path (malloced)
@param[out] pathy - return for y-coordiantes of path (malloced)
@returns Number of path points, -1 on fail.

*/
int astar2(unsigned char *binary, int width, int height, int sx, int sy, int ex, int ey, int **pathx, int **pathy)
{
	ASTAR *as;
	APOINT a, b, ap, np;
	int set, otherset;
	unsigned char neighbours[9];
	int i, ii;
	int j;
	int nx, ny;
	int Npa, Npb;
	int *pathax, *pathay, *pathbx, *pathby;
	int *tpathx, *tpathy;
	int answer = 0;

	as = malloc(sizeof(ASTAR));
	if (!as)
		goto out_of_memory;

	as->img = malloc(width * height);
	if (!as->img)
		goto out_of_memory;
	for (i = 0; i < width*height; i++)
		as->img[i] = binary[i] ? FILL : 0;

	as->img[sy*width + sx] |= ASET;
	as->img[ey*width + ex] |= BSET;

	as->q = priority_queue(sizeof(APOINT), astarcomp, as);
	a.x = sx;
	a.y = sy;
	a.fscore = 0;
	a.heuristic = diagonaldistance(sx, sy, ex, ey);
	priority_queue_insert(as->q, &a);

	b.x = ex;
	b.y = ey;
	b.fscore = 0;
	b.heuristic = diagonaldistance(sx, sy, ex, sy);
	priority_queue_insert(as->q, &b);

	while (priority_queue_getsize(as->q) > 0)
	{
		priority_queue_popfront(as->q, &ap);
		get3x3(neighbours, as->img, width,height, ap.x, ap.y, 0x0);
		set = neighbours[4] & SETMASK;
		if (set == ASET)
			otherset = BSET;
		else
			otherset = ASET;

		for (j = 0; j < 9; j++)
		{
			nx = ap.x + (j % 3) - 1;
			ny = ap.y + (j / 3) - 1;


			if ((neighbours[j] & FILLMASK) && (neighbours[j] & SETMASK) == 0)
			{
				double cost = 0.0;
				switch (j)
				{
				case 0: cost = 1.414;  as->img[ny*width + nx] |= (set | SOUTHEAST); break;
				case 1: cost = 1.0;    as->img[ny*width + nx] |= (set | SOUTH); break;
				case 2: cost = 1.414;  as->img[ny*width + nx] |= (set | SOUTHWEST); break;
				case 3: cost = 1.0;    as->img[ny*width + nx] |= (set | EAST); break;
				case 4: break;
				case 5: cost = 1.0;    as->img[ny*width + nx] |= (set | WEST); break;
				case 6: cost = 1.141;  as->img[ny*width + nx] |= (set | NORTHEAST); break;
				case 7: cost = 1.0;    as->img[ny*width + nx] |= (set | NORTH); break;
				case 8: cost = 1.414;  as->img[ny*width + nx] |= (set | NORTHWEST); break;
				}
				if (j != 4)
				{
					np.x = nx;
					np.y = ny;
					np.heuristic = diagonaldistance(np.x, np.y, ex, ey);
					np.fscore = ap.fscore + cost;
					priority_queue_insert(as->q, &np);
				}
			}
			else if ((neighbours[j] & SETMASK) == otherset)
			{
				Npa = traceback(as->img, width, height, ap.x, ap.y, &pathax, &pathay);
				Npb = traceback(as->img, width, height, nx, ny, &pathbx, &pathby);
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
	killpriority_queue(as->q);
	free(as->img);
   return answer;


out_of_memory:
   killpriority_queue(as->q);
   free(as->img);
	return -1;
}

/**
   Structure for the search area, or "balloon"
   
*/
typedef struct
{
	unsigned char *img;  /**< image, set up with direction data to source. */
	int width;           /**< image width. */
	int height;          /**< image height. */
	int *ax;             /**< boundary of a, x values. */
	int *ay;             /**< boundary of a, y values. */
	int Na;              /**< size of boundary a. */
	int *bx;             /**< boundary of b, x values. */
	int *by;             /**< boundary of b, y values. */
	int Nb;              /**< Nb size of boundary b. */
	int *pathx;          /**< path x co-ordinates. */
	int *pathy;          /**< path y co-ordinates. */
	int Npath;           /**< size of path. */
} BALLOON;

static BALLOON *balloon(unsigned char *binary, int width, int height, int sx, int sy, int ex, int ey);
static void killballoon(BALLOON *bal);
static int passa(BALLOON *bal);
static int passb(BALLOON *bal);
static int traceback(unsigned char *img, int width, int height, int x, int y, int **pathx, int **pathy);
static void reverse(int *x, int N);
static void get3x3(unsigned char *out, unsigned char *img, int width, int height, int x, int y, unsigned char border);

/**
  A star path finding algorithm.

  @param[in] binary - the bianry image
  @param width - image width
  @param height - image height
  @param sx - start point x-coordiante
  @param sy - start point y-coordiante
  @param ex - end point x coordiante
  @param ey - end point y coordiante
  @param[out] pathx - return for x-coordiantes of path (malloced)
  @param[out] pathy - return for y-coordiantes of path (malloced)
  @returns Number of path points, -1 on fail.

*/
int astar(unsigned char *binary, int width, int height, int sx, int sy, int ex, int ey, int **pathx, int **pathy)
{
	BALLOON *bal;
	int answer;
	int res;

	bal = balloon(binary, width, height, sx, sy, ex, ey);

	while (1)
	{
		res = passa(bal);
		if (res == -1)
			goto out_of_memory;
		if (res == 1 || res == 2)
			break;
		res = passb(bal);
		if (res == -1)
			goto out_of_memory;
		if (res == 1 || res == 2)
			break;
	}
	answer = bal->Npath;
	*pathx = bal->pathx;
	*pathy = bal->pathy;
	bal->pathx = 0;
	bal->pathy = 0;
	killballoon(bal);
	return answer;
out_of_memory:
	killballoon(bal);
	*pathx = 0;
	*pathy = 0;
	return -1;
}

static BALLOON *balloon(unsigned char *binary, int width, int height, int sx, int sy, int ex, int ey)
{
	BALLOON *bal;
	int i;

	bal = malloc(sizeof(BALLOON));
	if (!bal)
		goto out_of_memory;

	bal->img = malloc(width * height);
	if (!bal->img)
		goto out_of_memory;
	for (i = 0; i < width*height; i++)
		bal->img[i] = binary[i] ? FILL : 0;

	bal->width = width;
	bal->height = height;
	bal->ax = malloc(sizeof(int));
	bal->ay = malloc(sizeof(int));
	if (!bal->ax || !bal->ay)
		goto out_of_memory;
	bal->ax[0] = sx;
	bal->ay[0] = sy;
	bal->Na = 1;

	bal->bx = malloc(sizeof(int));
	bal->by = malloc(sizeof(int));
	if (!bal->bx || !bal->by)
		goto out_of_memory;
	bal->bx[0] = ex;
	bal->by[0] = ey;
	bal->Nb = 1;

	bal->img[sy*bal->width + sx] |= ASET;
	bal->img[ey*bal->width + ex] |= BSET;
	bal->pathx = 0;
	bal->pathy = 0;
	bal->Npath = 0;
	return bal;

out_of_memory:
	killballoon(bal);
	return 0;
}

static void killballoon(BALLOON *bal)
{
	if (bal)
	{
		free(bal->img);
		free(bal->ax);
		free(bal->ay);
		free(bal->bx);
		free(bal->by);
		free(bal->pathx);
		free(bal->pathy);
		free(bal);
	}
}

static int passa(BALLOON *bal)
{
	int i, j, ii;
	unsigned char neighbours[9];
	int nx, ny;
	int N = 0;
	int *newx = 0;
	int *newy = 0;
	int *pathax = 0;
	int *pathay = 0;
	int *pathbx = 0;
	int *pathby = 0;
	int *pathx = 0;
	int *pathy = 0;
	int Npa, Npb;

	newx = malloc(bal->Na * 8 * sizeof(int));
	newy = malloc(bal->Na * 8 * sizeof(int));
	if (!newx || !newy)
		goto out_of_memory;
	for (i = 0; i < bal->Na; i++)
	{
		get3x3(neighbours, bal->img, bal->width, bal->height, bal->ax[i], bal->ay[i], 0x0);
		for (j = 0; j < 9; j += 2)
			neighbours[j] = 0;
		for (j = 0; j < 9; j++)
		{
			nx = bal->ax[i] + (j%3) - 1;
			ny = bal->ay[i] + (j/3) - 1;
		

			if ((neighbours[j] & FILLMASK) && (neighbours[j] & SETMASK) == 0)
			{
				switch (j)
				{
				case 0: bal->img[ny*bal->width + nx] |= (ASET | SOUTHEAST); break;
				case 1: bal->img[ny*bal->width + nx] |= (ASET | SOUTH); break;
				case 2: bal->img[ny*bal->width + nx] |= (ASET | SOUTHWEST); break;
				case 3: bal->img[ny*bal->width + nx] |= (ASET | EAST); break;
				case 4: break;
				case 5: bal->img[ny*bal->width + nx] |= (ASET | WEST); break;
				case 6: bal->img[ny*bal->width + nx] |= (ASET | NORTHEAST); break;
				case 7: bal->img[ny*bal->width + nx] |= (ASET | NORTH); break;
				case 8: bal->img[ny*bal->width + nx] |= (ASET | NORTHWEST); break;
				}
				newx[N] = nx;
				newy[N] = ny;
				N++;
			}
			else if ( (neighbours[j] & SETMASK) == BSET)
			{
				Npa = traceback(bal->img, bal->width, bal->height, bal->ax[i], bal->ay[i], &pathax, &pathay);
				Npb = traceback(bal->img, bal->width, bal->height, nx, ny, &pathbx, &pathby);
				if (Npa == -1 || Npb == -1)
					goto out_of_memory;
				reverse(pathax, Npa);
				reverse(pathay, Npa);
				pathx = malloc((Npa + Npb) * sizeof(int));
				pathy = malloc((Npa + Npb) * sizeof(int));
				if (!pathx || !pathy)
					goto out_of_memory;
				for (ii = 0; ii < Npa; ii++)
				{
					pathx[ii] = pathax[ii];
					pathy[ii] = pathay[ii];
				}
				for (ii = 0; ii < Npb; ii++)
				{
					pathx[ii + Npa] = pathbx[ii];
					pathy[ii + Npa] = pathby[ii];
				}
				free(newx);
				free(newy);
				free(pathax);
				free(pathay);
				free(pathbx);
				free(pathby);
				bal->pathx = pathx;
				bal->pathy = pathy;
				bal->Npath = Npa + Npb;
				return 1;
			}
		}
	}
	if (N == 0)
	{
		free(newx);
		free(newy);
		return 2;
	}
	free(bal->ax);
	free(bal->ay);
	bal->ax = newx;
	bal->ay = newy;
	bal->Na = N;
	return  0;

out_of_memory:
	free(newx);
	free(newy);
	free(pathax);
	free(pathay);
	free(pathbx);
	free(pathby);
	free(pathx);
	free(pathy);
	return -1;
}

static int passb(BALLOON *bal)
{
	int i, j, ii;
	unsigned char neighbours[9];
	int nx, ny;
	int N = 0;
	int *newx = 0;
	int *newy = 0;
	int *pathax = 0;
	int *pathay = 0;
	int *pathbx = 0;
	int *pathby = 0;
	int *pathx = 0;
	int *pathy = 0;
	int Npa, Npb;

	newx = malloc(bal->Nb * 8 * sizeof(int));
	newy = malloc(bal->Nb * 8 * sizeof(int));
	if (!newx || !newy)
		goto out_of_memory;
	for (i = 0; i < bal->Nb; i++)
	{
		get3x3(neighbours, bal->img, bal->width, bal->height, bal->bx[i], bal->by[i], 0x0);
		for (j = 0; j < 9; j += 2)
			neighbours[j] = 0;
		for (j = 0; j < 9; j++)
		{
			nx = bal->bx[i] + (j % 3) - 1;
			ny = bal->by[i] + (j / 3) - 1;
			if ((neighbours[j] & FILLMASK) && (neighbours[j] & SETMASK) == 0)
			{
				switch (j)
				{
				case 0: bal->img[ny*bal->width + nx] |= (BSET | SOUTHEAST); break;
				case 1: bal->img[ny*bal->width + nx] |= (BSET | SOUTH); break;
				case 2: bal->img[ny*bal->width + nx] |= (BSET | SOUTHWEST); break;
				case 3: bal->img[ny*bal->width + nx] |= (BSET | EAST); break;
				case 4: break;
				case 5: bal->img[ny*bal->width + nx] |= (BSET | WEST); break;
				case 6: bal->img[ny*bal->width + nx] |= (BSET | NORTHEAST); break;
				case 7: bal->img[ny*bal->width + nx] |= (BSET | NORTH); break;
				case 8: bal->img[ny*bal->width + nx] |= (BSET | NORTHWEST); break;
				}
				newx[N] = nx;
				newy[N] = ny;
				N++;
			}
			else if ((neighbours[j] & SETMASK) == ASET)
			{
				Npa = traceback(bal->img, bal->width, bal->height, nx, ny, &pathax, &pathay);
				Npb = traceback(bal->img, bal->width, bal->height, bal->bx[i], bal->by[i], &pathbx, &pathby);
				if (Npa == -1 || Npb == -1)
					goto out_of_memory;
				reverse(pathax, Npa);
				reverse(pathay, Npa);
				pathx = malloc((Npa + Npb) * sizeof(int));
				pathy = malloc((Npa + Npb) * sizeof(int));
				if (!pathx || !pathy)
					goto out_of_memory;
				for (ii = 0; ii < Npa; ii++)
				{
					pathx[ii] = pathax[ii];
					pathy[ii] = pathay[ii];
				}
				for (ii = 0; ii < Npb; ii++)
				{
					pathx[ii + Npa] = pathbx[ii];
					pathy[ii + Npa] = pathby[ii];
				}
				free(newx);
				free(newy);
				free(pathax);
				free(pathay);
				free(pathbx);
				free(pathby);
				bal->pathx = pathx;
				bal->pathy = pathy;
				bal->Npath = Npa + Npb;
				return 1;
			}
		}
	}
	if (N == 0)
	{
		free(newx);
		free(newy);
		return 2;
	}
	free(bal->bx);
	free(bal->by);
	bal->bx = newx;
	bal->by = newy;
	bal->Nb = N;
	return  0;

out_of_memory:
	free(newx);
	free(newy);
	free(pathax);
	free(pathay);
	free(pathbx);
	free(pathby);
	free(pathx);
	free(pathy);
	return -1;
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


