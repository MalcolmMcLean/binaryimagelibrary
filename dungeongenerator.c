/**@file The random dungeon generator.
///
/// Starting with a stage of solid walls, it works like so:
///
/// 1. Place a number of randomly sized and positioned rooms. If a room
///    overlaps an existing room, it is discarded. Any remaining rooms are
///    carved out.
/// 2. Any remaining solid areas are filled in with mazes. The maze generator
///    will grow and fill in even odd-shaped areas, but will not touch any
///    rooms.
/// 3. The result of the previous two steps is a series of unconnected rooms
///    and mazes. We walk the stage and find every tile that can be a
///    "connector". This is a solid tile that is adjacent to two unconnected
///    regions.
/// 4. We randomly choose connectors and open them or place a door there until
///    all of the unconnected regions have been joined. There is also a slight
///    chance to carve a connector between two already-joined regions, so that
///    the dungeon isn't single connected.
/// 5. The mazes will have a lot of dead ends. Finally, we remove those by
///    repeatedly filling in any open tile that's closed on three sides. When
///    this is done, every corridor in a maze actually leads somewhere.
///
/// The end result of this is a multiply-connected dungeon with rooms and lots
/// of winding corridors.
/// Nicked from
/// http://journal.stuffwithstuff.com/2014/12/21/rooms-and-mazes/
///
/// Converted to C by Malcolm McLean
///*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct
{
	int x; 
	int y; 
	int width; 
	int height;
} RECT;

typedef struct Dungeon {
	int width;
	int height;
	int numRoomTries;

	/// The chance of adding a connector between two regions that have
	/// already been joined. Increasing this leads to more closely connected
	/// dungeons.
	int extraConnectorChance; // = > 20;

	/// Increasing this allows rooms to be larger.
	int roomExtraSize; // = > 0;

	int windingPercent; //  = > 0;

	RECT *rooms;
	int Nrooms;


	/// For each open position in the dungeon, the index of the connected region
	/// that that position is a part of.
	int *regions;
	//Array2D<int> _regions;

	/// The index of the current region being carved.
	int currentRegion; //= -1;

	unsigned char *Tiles;
} DUNGEON;

static DUNGEON *dungeon(int width, int height);
static void killdungeon(DUNGEON *dun);

static int connectRegions(DUNGEON *dun);
static int growMaze(DUNGEON *dun, int x, int y);
static int addRooms(DUNGEON *dun);
static void addJunction(DUNGEON *dun, int x, int y);
static void removeDeadEnds(DUNGEON *dun);
static int canCarve(DUNGEON *dun, int x, int y, int dx, int dy);
static void startRegion(DUNGEON *dun);
static void carve(DUNGEON *dun, int x, int y);
static int rects_overlap(RECT *a, RECT *b);
static void get3x3(unsigned char *out, unsigned char *binary, int width, int height, int x, int y, unsigned char border);

/**
  Create a dungeon as a binary image.

  @param width - dungeon width
  @param height - dungeon height
  @return Your dungeon, ready to populate with monsters.
*/
unsigned char *makedungeon(int width, int height)
{
	unsigned char *answer = 0;
	DUNGEON *dun;
	int x, y;
	int i;
	int err;

	dun = dungeon(width, height);
	if (!dun)
		goto out_of_memory;

	err = addRooms(dun);
	if (err < 0)
		goto out_of_memory;
	for (y = 1; y < dun->height-1; y += 2) {
		for (x = 1; x < dun->width -1; x += 2)
		{
			int Nneighbours = 0;
			if (dun->Tiles[y*dun->width + x] != 0) 
				continue;
			unsigned char neighbours[9];
			get3x3(neighbours, dun->Tiles, dun->width, dun->height, x, y, 0);
			for (i = 0; i < 9; i++)
				Nneighbours += neighbours[i];
			if (Nneighbours == 0)
			{
				err = growMaze(dun, x, y);
				if (err < 0)
					goto out_of_memory;
			}
		}
	}
	err = connectRegions(dun);
	if (err < 0)
		goto out_of_memory;
	removeDeadEnds(dun);

	answer = dun->Tiles;
	dun->Tiles = 0;

	return answer;
out_of_memory:
	killdungeon(dun);
	return 0;
}

/*
  Create the dungeon object
*/
static DUNGEON *dungeon(int width, int height)
{
	DUNGEON *dun;
	int i;

	dun = malloc(sizeof(DUNGEON));
	if (!dun)
		goto out_of_memory;
	dun->Tiles = 0;
	dun->regions = 0;
	dun->rooms = 0;

	dun->Tiles = malloc(width * height);
	if (!dun->Tiles)
		goto out_of_memory;
	memset(dun->Tiles, 0, width * height);
	dun->regions = malloc(width * height * sizeof(int));
	if (!dun->regions)
		goto out_of_memory;
	for (i = 0; i < width*height; i++)
		dun->regions[i] = 0;
	dun->currentRegion = 1;
	dun->width = width;
	dun->height = height;

	dun->rooms = 0;
	dun->Nrooms = 0;

	dun->numRoomTries = (width * height) / (16 * 16);
	dun->roomExtraSize = 8;

	dun->extraConnectorChance = 5;
	dun->windingPercent = 20;

	return dun;
out_of_memory:
	killdungeon(dun);
	return 0;
}

/*
  Destriy the dungeon object
*/
static void killdungeon(DUNGEON *dun)
{
	if (dun)
	{
		free(dun->regions);
		free(dun->rooms);
		free(dun->Tiles);
	}
}




	/// Implementation of the "growing tree" algorithm from here:
	/// http://www.astrolog.org/labyrnth/algrithm.htm.
	static int growMaze(DUNGEON *dun, int x, int y) 
	{
		int *cellsx = 0;
		int *cellsy = 0;
		int Ncells;

		int lastDirx;
		int lastDiry;
		void *temp;

		startRegion(dun);
		carve(dun, x, y);

		cellsx = malloc(1 * sizeof(int));
		if (!cellsx)
			goto out_of_memory;
		cellsy = malloc(1 * sizeof(int));
		if (!cellsy)
			goto out_of_memory;
		cellsx[0] = x;
		cellsy[0] = y;
		Ncells = 1;

		while (Ncells) {
			int cellx = cellsx[Ncells - 1];
			int celly = cellsy[Ncells - 1];
			int dx, dy;

			int unmadedx[4];
			int unmadedy[4];
			int Nunmade = 0;
			int i;
			// See which adjacent cells are open.
			for (i = 0; i < 4; i++)
			{
				dx = (i % 2) ? -1 : 1;
				dy = (i % 2) ? -1 : 1;
				if (i < 2)
					dy = 0;
				if (i >= 2)
					dx = 0;
				if (canCarve(dun, cellx, celly, dx, dy))
				{
					unmadedx[Nunmade] = dx;
					unmadedy[Nunmade] = dy;
					Nunmade++;
				}
			}

			if (Nunmade != 0) {
				// Based on how "windy" passages are, try to prefer carving in the
				// same direction.
				int contain_flag = 0;
				for (i = 0; i<Nunmade; i++)
				{
					if (unmadedx[i] == lastDirx && unmadedy[i] == lastDiry)
					{
						contain_flag = 1;
					}
				}

				if (contain_flag  && (rand() % 100) > dun->windingPercent) {
					dx = lastDirx;
					dy = lastDiry;
				}
				else {
					i = rand() % Nunmade;
					dx = unmadedx[i];
					dy = unmadedy[i];
				}

				carve(dun, cellx + dx, celly + dy);
				carve(dun, cellx + dx * 2, celly + dy *2);

				temp = realloc(cellsx, (Ncells+1) * sizeof(int));
				if (!temp)
					goto out_of_memory;
				cellsx = temp;
				if (!temp)
					goto out_of_memory;
				temp = realloc(cellsy, (Ncells +1) * sizeof(int));
				cellsy = temp;
				cellsx[Ncells] = cellx + dx*2;
				cellsy[Ncells] = celly + dy*2;
				Ncells++;
				lastDirx = dx;
				lastDiry = dy;
			}
			else {
				// No adjacent uncarved cells.
				Ncells--;

				// This path has ended.
				lastDirx = 0;
				lastDiry = 0;
			}
		}

		free(cellsx);
		free(cellsy);
		return 0;
	out_of_memory:
		free(cellsx);
		free(cellsy);
		return -1;

	}

	/// Places rooms ignoring the existing maze corridors.
	static int addRooms(DUNGEON *dun) 
	{
		int i, ii;
		void *temp;

		for (i = 0; i < dun->numRoomTries; i++) 
		{
			// Pick a random room size. The funny math here does two things:
			// - It makes sure rooms are odd-sized to line up with maze.
			// - It avoids creating rooms that are too rectangular: too tall and
			//   narrow or too wide and flat.
			// TODO: This isn't very flexible or tunable. Do something better here.
			int size = (rand() % (3 + dun->roomExtraSize) + 1) * 2 +1;
			int rectangularity = rand() % (1 + size/ 2) * 2;
			int width = size;
			int height = size;
			if (rand() % 2) {
				width += rectangularity;
			}
			else {
				height += rectangularity;
			}

			int x = rand() % ((dun->width - width)/ 2) * 2 +1;
			int y = rand() % ((dun->height - height) / 2) * 2 +1;

			RECT room; 

			room.x = x;
			room.y = y;
			room.width = width;
			room.height = height;

			int overlaps = 0;
			for (ii = 0; ii < dun->Nrooms;ii++) 
			{
				if (rects_overlap(&room, &dun->rooms[ii])) 
				{
					overlaps = 1;
					break;
				}
			}

			if (overlaps) continue;

			temp = realloc(dun->rooms, (dun->Nrooms + 1) *sizeof(RECT));
			if (!temp)
				goto out_of_memory;
			dun->rooms = temp;
			dun->rooms[dun->Nrooms] = room;
			dun->Nrooms++;

			startRegion(dun);

			int xx, yy;
			for (yy = y; yy < y + height; yy++)
			{
				for (xx = x; xx < x + width; xx++)
				{
					carve(dun, xx, yy);
				}
			}
		}
		return 0;
	out_of_memory:
		return -1;
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

	/*
	   The dungeon currently consists of rooms and maze-filled corridors
	   We need to connect all the rooms up. So we make a list of all
	   connecting pixels, chose one at random, and merge two regions.
	   Then we need to mark the two regions as merged.

	   (Both background and rooms are 4-connected, so a connector pixel
	   may connect only two regions).
	*/
	static int connectRegions(DUNGEON *dun) {

		int Nregions = dun->currentRegion +1;
		MERGETREE *mergelist;
		int x, y;
		int i;
		int Nmerges;
		CONNECTION *connlist = 0;
		int Nconns = 0;
		int Nopenregions;
		void *temp;

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

		for (y = 1; y < dun->height - 1; y++)
			for (x = 1; x < dun->width - 1; x++)
			{
				int reg4[4];
				int Nreg4 = 0;
				int connection = 0;
				CONNECTION conn;

				if (dun->Tiles[y*dun->width + x] == 0)
				{
					if (dun->regions[y*dun->width + x] == 0)
					{
						if (dun->Tiles[y*dun->width + x + 1] != 0)
							reg4[Nreg4++] = dun->regions[y*dun->width + x + 1];
						if (dun->Tiles[y*dun->width + x - 1] != 0)
							reg4[Nreg4++] = dun->regions[y*dun->width + x - 1];
						if (dun->Tiles[(y + 1)*dun->width + x] != 0)
							reg4[Nreg4++] = dun->regions[(y + 1)*dun->width + x];
						if (dun->Tiles[(y - 1)*dun->width + x] != 0)
							reg4[Nreg4++] = dun->regions[(y - 1)*dun->width + x];
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

			index = rand() % Nconns;
			addJunction(dun, connlist[index].x, connlist[index].y);
			err = merge(mergelist, connlist[index].regiona, connlist[index].regionb, Nmerges);
			if (err != -1)
				Nmerges++;
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

				if (kill && (rand() % 100) < dun->extraConnectorChance)
				{
					addJunction(dun, connlist[i].x, connlist[i].y);
				}

				if (!kill)
					connlist[j++] = connlist[i];
			}
			Nconns = j;

		}

		free(connlist);
		free(mergelist);
		return 0;
	out_of_memory:
		free(connlist);
		free(mergelist);
		return -1;



		
	}

	/*
	  Put in a door (mark the regions specially?)
	*/
	static void addJunction(DUNGEON *dun, int x, int y) 
	{
		dun->Tiles[y*dun->width + x] = 1;
		/*
		if (rng.oneIn(4)) {
			setTile(pos, rng.oneIn(3) ? Tiles.openDoor : Tiles.floor);
		}
		else {
			setTile(pos, Tiles.closedDoor);
		}
		*/
	}

	/*
	  prune dead end points
	*/
	static void removeDeadEnds(DUNGEON *dun)
	{
		int done = 0;

		unsigned char neighbours[9];
		int x, y;
		int Nneighbours;
		int i;

		while (!done) {

			done = 1;

			for (y = 1; y < dun->height - 1; y++)
			{
				for (x = 1; x < dun->width - 1; x++)
				{
					if (dun->Tiles[y*dun->width + x])
					{
						Nneighbours = 0;
						get3x3(neighbours, dun->Tiles, dun->width, dun->height, x, y, 0);
						for (i = 1; i < 9; i+=2)
						{
							if (neighbours[i])
								Nneighbours++;
						}
						if (Nneighbours == 1)
						{
							dun->Tiles[y*dun->width + x] = 0;
							dun->regions[y*dun->width + x] = 0;
							done = 0;
						}

					}
					
				}
			}
		
		}
	}

	/// Gets whether or not an opening can be carved from the given starting
	/// [Cell] at [pos] to the adjacent Cell facing [direction]. Returns `true`
	/// if the starting Cell is in bounds and the destination Cell is filled
	/// (or out of bounds).
	static int canCarve(DUNGEON *dun, int x, int y, int dx, int dy) {
		int tx, ty;

		tx = x + dx * 3;
		ty = y + dy * 3;

		if (tx < 0 || ty < 0 || tx >= dun->width || ty >= dun->height)
			return 0;

		tx = x + dx;
		ty = y + dy;
		if (dun->Tiles[ty*dun->width + tx] == 1)
			return 0;

		
		// Destination must not be open.
		tx = x + dx * 2;
		ty = y + dy * 2;

		
		if (dun->Tiles[ty*dun->width + tx] == 1)
			return 0;

		return 1;
	}

	static void startRegion(DUNGEON *dun) {
		dun->currentRegion++;
	}

	static void carve(DUNGEON *dun, int x, int y) 
	{
		dun->Tiles[y*dun->width + x] = 1;
		dun->regions[y*dun->width+x] = dun->currentRegion;
	}

	static int rects_overlap(RECT *a, RECT *b)
	{
		if (a->x + a->width <= b->x)
			return 0;
		if (b->x + b->width <= a->x)
			return 0;
		if (a->y + a->height <= b->y)
			return 0;
		if (b->y + b->height <= a->y)
			return 0;

		return 1;
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