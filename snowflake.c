/*
  makes a nice snowflake type image by iteratively docking particles
    to a growing structure.

  By Malcolm McLean

*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <assert.h>


#define uniform() (rand()/(RAND_MAX + 1.0))
#define PI 3.14159265358979

typedef struct
{
	int width;
	int height;
	int cx;        /* seed x, y co-ordinates */
	int cy;
    int maxrad2;   /* maximum radius of flake, squared */
	int maxlost2;  /* radius^2 at which particle lost */
	unsigned char *grid;  
	int px;  /* particle x, y co-ordinates */
	int py;
	int px0; /* initial x, y, co-ordinates for reset */
	int py0;
} PLANE;

PLANE *plane(int width, int height);
void killplane(PLANE *pl);
void addparticle(PLANE *pl);
int stepparticle(PLANE *pl);
int collision(PLANE *pl, int x, int y);


int collision2(PLANE *pl, int x, int y);
void get8(unsigned char *out, unsigned char *grid, int width, int height, int x, int y);

/*
  Object constructor *
  width - plane width
  height - plane height
*/
PLANE *plane(int width, int height)
{
  PLANE *answer;

  answer = malloc(sizeof(PLANE));
  if(!answer)
	  goto out_of_memory;
  answer->grid = malloc(width * height);
  if(!answer->grid)
	  goto out_of_memory;
  answer->width = width;
  answer->height = height;
  answer->cx = width/2;
  answer->cy = height/2;
  memset(answer->grid, 0, width * height);
  answer->grid[answer->cy * answer->width + answer->cx] = 1;
  answer->maxrad2 = 1;
  answer->maxlost2 = 14*14;
  return answer;
out_of_memory:
  killplane(answer);
  return 0;
}

/* destructor */
void killplane(PLANE *pl)
{
  if(pl)
  {
    free(pl->grid);
	free(pl);
  }
}

/*
  Insert a new particle into the plane
  Particles appears on tip of circle around the growing flake
*/
void addparticle(PLANE *pl)
{
  int px, py;
  double theta;
  double d;

  theta = uniform() * 2 * PI;
  d = (uniform() * 10) + sqrt(pl->maxrad2);
  px = (int) (cos(theta) * d);
  py = (int) (sin(theta) * d);

  pl->px = px + pl->cx;
  pl->py = py + pl->cy;
  pl->px0 = pl->px;
  pl->py0 = pl->py;

}

/*
  step the particle one dx, dy move of one square
  returns 1 if it docks, 0 if it doesn't
*/
int stepparticle(PLANE *pl)
{
	int dx;
	int dy;
	int d2;

	dx = (int) (uniform() * 3) - 1;
	dy = (int) (uniform() * 3) - 1;

	if(collision2(pl, pl->px + dx, pl->py + dy))
	{
	  pl->grid[(pl->py +dy) * pl->width + pl->px + dx] = 1;
      d2 = (pl->px - pl->cx)*(pl->px - pl->cx) + (pl->py - pl->cy)*(pl->py - pl->cy);
	  if(d2 > pl->maxrad2)
	  {
		  pl->maxrad2 = d2;
		  pl->maxlost2 = (int) ((sqrt(d2) + 13)*(sqrt(d2) + 13));
	  }
 
	  return 1;
	}
	if( (pl->px + dx - pl->cx) * (pl->px + dx - pl->cx) +
		(pl->py + dy - pl->cy) * (pl->py + dy - pl->cy) > pl->maxlost2)
	    {
		   pl->px = pl->px0;
		   pl->py = pl->py0;
	    }
	else
	{
	  pl->px += dx;
	  pl->py += dy;
	}

	return 0;
}

/*
  does particle collide with a docked particle?
*/
int collision(PLANE *pl, int x, int y)
{
  if(x < 0 || x >= pl->width || y < 0 || y >= pl->height)
	  return 0;
  return pl->grid[y * pl->width + x];
}

int collision2(PLANE *pl, int x, int y)
{
	int i;
	unsigned char buff[8];

	if(x < 0 || x >= pl->width || y < 0 || y >= pl->height)
	  return 0;
	get8(buff, pl->grid, pl->width, pl->height, x, y);
	for(i=0;i<8;i++)
	  if(buff[i])
		  return 1;
	return 0;
	
}

/*
  get neighbouring pixels of x, y
  Params: out - return for 8 pixels
          grid - the grid
		  width - grid width
		  height - grid height
		  x, y - x y to test
*/
void get8(unsigned char *out, unsigned char *grid, int width, int height, int x, int y)
{
	out[0] = (x > 0 && y > 0) ? grid[(y-1)*width+x-1] : 0;
	out[1] = (         y > 0) ? grid[(y-1)*width+x]   : 0;
	out[2] = (x < width-1 && y > 0) ? grid[(y-1)*width+x+1] : 0;

	out[3] = (x > 0)                ? grid[y*width+x-1] : 0;
	out[4] = (x < width-1)          ? grid[y*width+x+1] : 0;

	out[5] = (x > 0 && y < height -1) ? grid[(y+1)*width+x-1] : 0;
	out[6] = (         y < height -1) ? grid[(y+1)*width+x]   : 0;
	out[7] = (x < width-1 && y < height-1) ? grid[(y+1)*width+x+1] : 0;
}

void usage(void)
{
  printf("Creates a nice snowflake effect from docking particles\n");
  printf("Usage : cystaldock <width> <outfile.gif>\n");
  printf(" width - dimesion of output image\n");
  printf(" outfile.gif - output file\n");
  exit(EXIT_FAILURE);
}

void snowflakedock(unsigned char *binary, int width, int height)
{
	PLANE *pl = 0 ;
	int N;
	int step;
	int ans;
	int Ndocked = 0;

	assert(width == height);

	N = width;
	pl = plane(N, N);
	if (!pl)
		goto out_of_memory;

	memcpy(pl->grid, binary, N * N);
	while (1)
	{
		addparticle(pl);
		step = 0;
		while ((ans = stepparticle(pl)) == 0)
			step++;
		if (ans == 1)
		{
			Ndocked++;
			printf("%d docked \n", Ndocked);
			if (pl->maxrad2 > (N / 2 - 1)*(N / 2 - 1))
				break;
		}
	}
	memcpy(binary, pl->grid, N * N);
	killplane(pl);
	return 0;

out_of_memory:
	killplane(pl);
	return -1;
}
/*
int main(int argc, char **argv)
{
  PLANE *pl;
  int ans;
  int step;
  int Ndocked = 0;
  unsigned char pal[6] = {0, 0, 0, 255, 255, 255}; 
  int N;
  int err;

  if(argc != 3)
    usage();
  N = atoi(argv[1]);
  if(N < 1)
	  usage();

  srand(time(0));
  printf("crystaldock\n");

  pl = plane(N, N);

  while(1)
  {
	  addparticle(pl);
	  step = 0;
	  while( (ans = stepparticle(pl)) == 0)
	    step++;
	  if(ans == 1)
	  {
	    Ndocked++;
		printf("%d docked \n", Ndocked);
		if( pl->maxrad2 > (N/2-1)*(N/2-1) )
			break;
	  }
  }

  err = savegif(argv[2], pl->grid, pl->width, pl->height, pal, 2, -1, 0, 0);
  if(err)
  {
    fprintf(stderr, "Error saving file %s\n", argv[2]);
    exit(EXIT_FAILURE);
  }

  killplane(pl);
  return 0;
}
*/