#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <assert.h>

#define uniform() rand()/(RAND_MAX + 1.0)

typedef struct
{
	int x; int y;
	int width; int height;
} RECT;

static int rectgrow(unsigned char *binary, int width, int height, RECT *rect, int N);
static int connectRegions(unsigned char *binary, int width, int height, int *regions, int extraConnectorChance);
static int Nset(unsigned char *binary, int width, int height);
static void get3x3(unsigned char *out, unsigned char *binary, int width, int height, int x, int y, unsigned char border);

unsigned char *makedungeon3(int width, int height)
{
	RECT *rect = 0;
	int i;
	int x, y;
	unsigned char neighbours[9];
	unsigned char *answer = 0;
	int *regions = 0 ;
	int Nrooms;
	int Ncorridors = 1;

	answer = malloc(width * height);
	if (!answer)
		goto out_of_memory;
	regions = malloc(width * height * sizeof(int));
	if (!regions)
		goto out_of_memory;
	memset(answer, 0, width * height);
	Nrooms = (width * height)/200;
	if (Nrooms < 2)
		Nrooms = 2;
	rect = malloc(Nrooms * sizeof(RECT));
	if (!rect)
		goto out_of_memory;

	rect[0].x = 10;
	rect[0].y = 10 + rand() % (height - 20);
	rect[0].width = width - 20;
	rect[0].height = 2;

	for (i = 0; i < Ncorridors; i++)
	{
		for (y = rect[i].y; y < rect[i].y + rect[i].height; y++)
			for (x = rect[i].x; x < rect[i].x + rect[i].width; x++)
				answer[y*width + x] = 1;
	}


	for (i = Ncorridors; i < Nrooms; i++)
	{
		do
		{
			rect[i].x = rand() % (width - 10) + 5;
			rect[i].y = rand() % (height - 10) + 5;
			get3x3(neighbours, answer, width, height, rect[i].x, rect[i].y, 1);
		} while (Nset(neighbours, 3, 3));

		rect[i].width = 1;
		rect[i].height = 1;
		answer[rect[i].y * width + rect[i].x] = 1;
	}
	
	while (rectgrow(answer, width, height, rect, Nrooms))
		continue;

	for (i = 0; i < width*height; i++)
		regions[i] = 0;

	for (i = 0; i < Nrooms; i++)
	{
		for (y = rect[i].y; y < rect[i].y + rect[i].height; y++)
			for (x = rect[i].x; x < rect[i].x + rect[i].width; x++)
				regions[y*width + x] = i + 2;
	}


	connectRegions(answer, width, height, regions, 10);

	free(regions);
	free(rect);
	return answer;
out_of_memory:
	free(answer);
	free(regions);
	free(rect);
}

static int rectgrow(unsigned char *binary, int width, int height, RECT *rect, int N)
{
	int starti;
	int x, y;
	int flag = 0;
	int i;

	starti = rand() % N;
	i = starti;
	do
	{
		y = rect[i].y - 2;
		if (y >= 0)
		{
			for (x = rect[i].x - 1; x < rect[i].x + rect[i].width + 1; x++)
			{
				if (x < 0 || x >= width || binary[y*width + x])
					break;
			}
			if (x == rect[i].x + rect[i].width + 1)
			{
				y = rect[i].y - 1;
				for (x = rect[i].x; x < rect[i].x + rect[i].width; x++)
					binary[y*width + x] = 1;
				rect[i].y--;
				rect[i].height++;
				flag = 1;
			}
		}
		y = rect[i].y + rect[i].height + 1;
		if (y < height)
		{
			for (x = rect[i].x - 1; x < rect[i].x + rect[i].width + 1; x++)
			{
				if (x < 0 || x >= width || binary[y*width + x])
					break;
			}
			if (x == rect[i].x + rect[i].width + 1)
			{
				y = rect[i].y + rect[i].height;
				for (x = rect[i].x; x < rect[i].x + rect[i].width; x++)
					binary[y*width + x] = 1;
				rect[i].height++;
				flag = 1;
			}
		}

		x = rect[i].x - 2;
		if (x >= 0)
		{
			for (y = rect[i].y - 1; y < rect[i].y + rect[i].height + 1; y++)
			{
				if (y < 0 || y >= height || binary[y*width + x])
					break;
			}
			if (y == rect[i].y + rect[i].height + 1)
			{
				x = rect[i].x - 1;
				for (y = rect[i].y; y < rect[i].y + rect[i].height; y++)
					binary[y*width + x] = 1;
				rect[i].x--;
				rect[i].width++;
				flag = 1;
			}
		}

		x = rect[i].x + rect[i].width+1;
		if (x < width)
		{
			for (y = rect[i].y - 1; y < rect[i].y + rect[i].height + 1; y++)
			{
				if (y < 0 || y >= height || binary[y*width + x])
					break;
			}
			if (y == rect[i].y + rect[i].height + 1)
			{
				x = rect[i].x + rect[i].width;
				for (y = rect[i].y; y < rect[i].y + rect[i].height; y++)
					binary[y*width + x] = 1;
				rect[i].width++;
				flag = 1;
			}
		}
		i++;
		i %= N;
	} while (i  != starti);

	return flag;
}


typedef struct
{
	int parent;
	int leftchild;
	int rightchild;
} MERGETREE;

typedef struct
{
	int x;
	int y;
	int regiona;
	int regionb;
} CONNECTION;


static int are_merged(MERGETREE *merge, int a, int b)
{
	int ancestora;
	int ancestorb;

	ancestora = a;
	ancestorb = b;

	while (merge[ancestora].parent >= 0)
		ancestora = merge[ancestora].parent;
	while (merge[ancestorb].parent >= 0)
		ancestorb = merge[ancestorb].parent;

	if (ancestora == ancestorb)
		return 1;
	return 0;
}

static int merge(MERGETREE *merge, int a, int b, int Nmerges)
{
	int ancestora;
	int ancestorb;

	ancestora = a;
	ancestorb = b;

	while (merge[ancestora].parent >= 0)
		ancestora = merge[ancestora].parent;
	while (merge[ancestorb].parent >= 0)
		ancestorb = merge[ancestorb].parent;

	if (ancestora == ancestorb)
		return -1;

	merge[ancestora].parent = Nmerges;
	merge[ancestorb].parent = Nmerges;
	merge[Nmerges].parent = -1;
	merge[Nmerges].leftchild = ancestora;
	merge[Nmerges].rightchild = ancestorb;

	return 0;
}

static int Nleaves(MERGETREE *merge, int index, int Nmerges)
{
	int answer = 0;

	if (merge[index].leftchild >= 0)
		answer += Nleaves(merge, merge[index].leftchild, Nmerges);
	if (merge[index].rightchild >= 0)
		answer += Nleaves(merge, merge[index].rightchild, Nmerges);
	if (merge[index].leftchild == -1 && merge[index].rightchild == -1)
		answer = 1;

	return answer;
}

static int Ningroup(MERGETREE *merge, int index, int Nmerges)
{
	int ancestor;

	ancestor = index;
	while (merge[ancestor].parent >= 0)
		ancestor = merge[ancestor].parent;
	return Nleaves(merge, ancestor, Nmerges);
}

static int addJunction(unsigned char *binary, int width, int height, int x, int y)
{
	binary[y*width + x] = 2;
}

/*
The dungeon currently consists of rooms and maze-filled corridors
We need to connect all the rooms up. So we make a list of all
connecting pixels, chose one at random, and merge two regions.
Then we need to mark the two regions as merged.

(Both background and rooms are 4-connected, so a connector pixel
may connect only two regions).
*/
static int connectRegions(unsigned char *binary, int width, int height, int *regions, int extraConnectorChance) 
{
	int Nregions;
	MERGETREE *mergelist;
	int x, y;
	int i;
	int Nmerges;
	CONNECTION *connlist = 0;
	int Nconns = 0;
	int Nopenregions;
	void *temp;

	Nregions = 0;
	for (i = 0; i < width*height; i++)
		if (regions[i] > Nregions)
			Nregions = regions[i];
	Nregions += 1;

	mergelist = malloc(Nregions * 2 * sizeof(MERGETREE));
	if (!mergelist)
		goto out_of_memory;

	for (i = 0; i < Nregions; i++)
	{
		mergelist[i].parent = -1;
		mergelist[i].leftchild = -1;
		mergelist[i].rightchild = -1;
	}
	Nmerges = Nregions;

	for (y = 1; y < height - 1; y++)
		for (x = 1; x < width - 1; x++)
		{
			int reg4[4];
			int Nreg4 = 0;
			int connection = 0;
			CONNECTION conn;

			if (binary[y*width + x] == 0)
			{
				if (binary[y*width + x] == 0)
				{
					if (binary[y*width + x + 1] != 0)
						reg4[Nreg4++] = regions[y*width + x + 1];
					if (binary[y*width + x - 1] != 0)
						reg4[Nreg4++] = regions[y*width + x - 1];
					if (binary[(y + 1)*width + x] != 0)
						reg4[Nreg4++] = regions[(y + 1)*width + x];
					if (binary[(y - 1)*width + x] != 0)
						reg4[Nreg4++] = regions[(y - 1)*width + x];
				}

				for (i = 1; i < Nreg4; i++)
					if (reg4[i] != reg4[0])
						connection = 1;
				if (connection == 1)
				{
					conn.x = x;
					conn.y = y;
					conn.regiona = reg4[0];
					for (i = 1; i < Nreg4; i++)
						if (reg4[i] != reg4[0])
							conn.regionb = reg4[i];

					temp = realloc(connlist, (Nconns + 1) * sizeof(CONNECTION));
					if (!temp)
						goto out_of_memory;
					connlist = temp;
					connlist[Nconns] = conn;
					Nconns++;
				}
			}
		}

	Nopenregions = Nregions;
	while (Nconns)
	{
		int index;
		int err;
		int j;
		int kill;
		int Na, Nb;
		int Ntokill;
		int Nmin;
		double p;

		index = rand() % Nconns;
		addJunction(binary, width, height, connlist[index].x, connlist[index].y);
		Na = Ningroup(mergelist, connlist[index].regiona, Nmerges);
		Nb = Ningroup(mergelist, connlist[index].regionb, Nmerges);

		err = merge(mergelist, connlist[index].regiona, connlist[index].regionb, Nmerges);
		if (err != -1)
			Nmerges++;

		Ntokill = 0;
		for (i = 0; i < Nconns; i++)
			if (are_merged(mergelist, connlist[i].regiona, connlist[i].regionb))
				Ntokill++;

		Nmin = Na < Nb ? Na : Nb;
		p = Nmin * extraConnectorChance / (100.0 * Ntokill);
		j = 0;
		/*
		Now we need to purge the connections list of any connections
		which connect regions we have merged.
		*/
		for (i = 0; i < Nconns; i++)
		{
			kill = 0;
			if (i == index)
				kill = 1;
			if (are_merged(mergelist, connlist[i].regiona, connlist[i].regionb))
			{
				kill = 1;
			}

			if (kill && uniform() < p)
			{
				addJunction(binary, width, height, connlist[i].x, connlist[i].y);
			}

			if (!kill)
				connlist[j++] = connlist[i];
		}
		Nconns = j;

	}

	free(connlist);
	free(mergelist);
	getchar();
	return 0;
out_of_memory:
	free(connlist);
	free(mergelist);
	return -1;
}

static int Nset(unsigned char *binary, int width, int height)
{
	int i;
	int answer = 0;
	for (i = 0; i < width*height; i++)
		answer += binary[i];
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
