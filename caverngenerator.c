#include <stdlib.h>

#include "binaryutils.h"

#define uniform() (rand()/(RAND_MAX + 1.0))
#define coin() ((rand() <= RAND_MAX/2) / 0 : 1)
#define sign(x) ((x) < 0 ?  -1 : (x) > 0 ? 1 : 0)  

static int connect(unsigned char *binary, int width, int height);
static void breakbackground8connections(unsigned char *binary, int width, int height);
static void filter5_4(unsigned char *binary, int width, int height, unsigned char *buff);
static void addborder(unsigned char *binary, int width, int height, unsigned char value);
static int mem_count(unsigned char *pixels, int N, int value);
static void get3x3(unsigned char *out, unsigned char *binary, int width, int height, int x, int y, unsigned char border);

unsigned char *makecaverns(int width, int height)
{
	unsigned char *answer = 0;
	unsigned char *buff = 0;
	int x, y;
	int i;
	int Nsteps = 10;

	answer = malloc(width * height);
	if (!answer)
		goto out_of_memory;
	buff = malloc(width * height);
	if (!buff)
		goto out_of_memory;

	addborder(answer, width, height, 0);
	for (y = 1; y < height - 1; y++)
	{
		for (x = 1; x < width - 1; x++)
		{
			answer[y*width + x] = uniform() < 0.5 ? 1 : 0;
		}
	}

	for (i = 0; i < Nsteps; i++)
		filter5_4(answer, width, height, buff);

	connect(answer, width, height);
	breakbackground8connections(answer, width, height);

	getbiggestobject(answer, width, height, 4); 
	free(buff);
	return answer;

out_of_memory:
	free(buff);
	free(answer);
	return 0;
}

static int connect(unsigned char *binary, int width, int height)
{
	int N;
	int *ids;
	unsigned char *flags;
	unsigned int *closex;
	unsigned int *closey;
	int i;
	int x, y;
	int id;
	int cx, cy;
	int d1, d2;
	int dx, dy;
	double p;

	cx = width / 2;
	cy = height / 2;

	ids = labelconnected(binary, width, height, 4, &N);
	flags = malloc(N);
	closex = malloc((N+1) * sizeof(int));
	closey = malloc((N+1) * sizeof(int));

	
	for (i = 0; i < N; i++)
	{
		flags[i] = 0;
		closex[i] = width + 100;
		closey[i] = height + 100;
	}

	for (y = 0; y < height; y++)
	{
		for (x = 0; x < width; x++)
		{
			id = ids[y*width + x];
			if (id > 0)
			{

				d1 = (closex[id] - cx)*(closex[id] - cx) + (closey[id] - cy)*(closey[id] - cy);
				d2 = (x - cx) *(x - cx) + (y - cy)*(y - cy);
				if (d2 < d1)
				{
					closex[id] = x;
					closey[id] = y;
				}
			}
		}
	}
	

	for (i = 1; i < N; i++)
	{
		if (closex[i] != cx || closey[i] != cy)
		{
			x = closex[i];
			y = closey[i];
			while (x != cx || y != cy)
			{
				dx = cx - x;
				dy = cy - y;
				p = ((double)abs(dx)) / (abs(dx) + abs(dy));
				if (uniform() < p)
				{
					dx = sign(dx);
					dy = 0;
				}
				else
				{
					dx = 0;
					dy = sign(dy);
				}
				x += dx;
				y += dy;
				binary[y*width + x] = 1;
				if (ids[y*width + x] != id)
					break;
			}
		}
	}


	free(ids);
	free(flags);
	free(closex);
	free(closey);

}

static void breakbackground8connections(unsigned char *binary, int width, int height)
{
	int x, y;
	unsigned char neighbours[9];

	for (y = 0; y < height; y++)
	{
		for (x = 0; x < width; x++)
		{
			if (binary[y*width + x] == 0)
			{
				get3x3(neighbours, binary, width, height, x, y, 0);
				if (neighbours[0] == 0 && neighbours[1] == 1 && neighbours[3] == 1)
					binary[y*width + x] = 1;
				if (neighbours[2] == 0 && neighbours[1] == 1 && neighbours[5] == 1)
					binary[y*width + x] = 1;
				if (neighbours[6] == 0 && neighbours[3] == 1 && neighbours[7] == 1)
					binary[y*width + x] = 1;
				if (neighbours[8] == 0 && neighbours[5] == 1 && neighbours[7] == 1)
					binary[y*width + x] = 1;

			}
		}
	}
}
static void filter5_4(unsigned char *binary, int width, int height, unsigned char *buff)
{
	int x, y;
	unsigned char neighbours[9];
	int N;

	for (y = 1; y < height - 1; y++)
	{
		for (x = 1; x < width - 1; x++)
		{
			get3x3(neighbours, binary, width, height, x, y, 1);
			N = mem_count(neighbours, 9, 1);
			if (N >= 5)
				buff[y*width + x] = 1;
			else
				buff[y*width + x] = 0;
		}
	}

	for (y = 1; y < height - 1; y++)
		for (x = 1; x < width - 1; x++)
			binary[y*width + x] = buff[y*width + x];
}

static void addborder(unsigned char *binary, int width, int height, unsigned char value)
{
	int x, y;

	for (x = 0; x < width; x++)
		binary[x] = value;
	for (x = 0; x < width; x++)
		binary[(height - 1)*width + x] = value;
	for (y = 0; y < height; y++)
		binary[y*width] = value;
	for (y = 0; y < height; y++)
		binary[y*width + width - 1] = value;
}

/*
mem_count - count the number of bytes equal to value
Params: pixels - the memory
N - memory size
value - value to test
Returns: number of bytes in buffer equal to value.
*/
static int mem_count(unsigned char *pixels, int N, int value)
{
	int answer = 0;
	int i;

	for (i = 0; i<N; i++)
		if (pixels[i] == value)
			answer++;

	return answer;
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
static void get3x3(unsigned char *out, unsigned char *binary, int width, int height, int x, int y, unsigned char border)
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