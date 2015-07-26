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
	unsigned char *img;
	int width;
	int height;
	int *ax;
	int *ay;
	int Na;
	int *bx;
	int *by;
	int Nb;
	int *pathx;
	int *pathy;
	int Npath;
} BALLOON;

static BALLOON *balloon(unsigned char *binary, int width, int height, int sx, int sy, int ex, int ey);
static void killballoon(BALLOON *bal);
static int passa(BALLOON *bal);
static int passb(BALLOON *bal);
static int traceback(unsigned char *img, int width, int height, int x, int y, int **pathx, int **pathy);
static void reverse(int *x, int N);
static void get3x3(unsigned char *out, unsigned char *img, int width, int height, int x, int y, unsigned char border);


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
		for (j = 0; j < 9; j++)
		{
			nx = bal->ax[i] + (j%3) -1;
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

/*
  get the Otusu threshold for image segmentation
  Params: gray - the grayscale image
          width - image width
          height - uimage height
  Returns: threshold at which to split pixels into foreground and
           background.
 */
int getOtsuthreshold(unsigned char *grey, int width, int height)
{
  int hist[256] = {0};
  int wB = 0;
  int wF;
  float mB, mF;
  float sum = 0;
  float sumB = 0;
  float varBetween;
  float varMax = 0.0f;
  int answer = 0;
  int i;
  int k;
  
  for(i=0;i<width*height;i++)
    hist[grey[i]]++;
 
  /* sum of all (for means) */
  for (k=0 ; k<256 ; k++) 
       sum += k * hist[k];

  for(k=0;k<256;k++)
  {
     wB += hist[k];               
     if (wB == 0) 
         continue;

     wF = width*height - wB;            
     if (wF == 0) 
       break;

     sumB += (float) (k * hist[k]);

     mB = sumB / wB;            /* Mean Background */
     mF = (sum - sumB) / wF;    /* Mean Foreground */

     /* Calculate Between Class Variance */
     varBetween = (float)wB * (float)wF * (mB - mF) * (mB - mF);

     /* Check if new maximum found */
     if (varBetween > varMax) 
     {
       varMax = varBetween;
       answer = k;
     }

  }
  return answer;
}
