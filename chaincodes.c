#include <stdlib.h>
#include <assert.h>

static void getclockwise(int *ncwise, unsigned char *bcwise, unsigned char *neighbours, int b);
static void get3x3(unsigned char *out, unsigned char *binary, int width, int height, int x, int y, unsigned char border);

int binary_followboundary(unsigned char *binary, int width, int height, int **pathx, int **pathy)
{
  int x, y;
  int cx, cy;
  int *buffer = 0;
  int *temp;
  unsigned char neighbours[9];
  int ncwise[8];
  unsigned char bcwise[8];
  int i;
  int N = 0;
  int bufflen = 0;
  int b;

  if(pathx)
    *pathx = 0;
  if(pathy)
    *pathy = 0;
  for(y=0;y<height;y++)
  {
    for(x=0;x<width;x++)
    {
      if(binary[y*width+x])
        goto foundfirst;
    }
  }
 
  return 0;
 foundfirst:
  cx = x;
  cy = y;
  b = 5;

  get3x3(neighbours, binary, width, height, cx, cy, 0);

  do
  {
    if(bufflen < (N+1) *2)
    {
      temp  = realloc(buffer, (bufflen * 2 + 64) * sizeof(int));
      if(!temp)
        goto error_exit;
      buffer = temp;
      bufflen = bufflen * 2 + 64;
    }
      
    buffer[N*2] = cx;
    buffer[N*2+1] = cy;
    N++;
    
    get3x3(neighbours, binary, width, height, cx, cy, 0);
    getclockwise(ncwise, bcwise, neighbours, b);

    for(i=0;i<8;i++)
      if(bcwise[i])
        break;
    if(i == 8)
      break;

    if(i)
      b = ncwise[i-1];
    else
      b = ncwise[7];

    cx = cx + (ncwise[i] % 3) -1;
    cy = cy + (ncwise[i] /3) -1;
    }
    while(cx != x || cy != y);
    
  if(pathx)
  {
    *pathx = malloc(N * sizeof(int));
	if (!*pathx)
		goto error_exit;
    for(i=0;i<N;i++)
      (*pathx)[i] = buffer[i*2];
  }
  if(pathy)
  {
    *pathy = malloc(N * sizeof(int));
	if (!*pathy)
		goto error_exit;
    for(i=0;i<N;i++)
      (*pathy)[i] = buffer[i*2+1];
  }
  free(buffer);
  return N;
  
 error_exit:
  if(pathx)
  {
    free(pathx);
    *pathx = 0;
  }
  if(pathy)
  {
    free(pathy);
    *pathy = 0;
  }
  free(buffer);
  return -1;
  }

/**
  Get a chain code from a binary image.

  Chain codes go like this:

                 3 2 1
				 4 X 0
				 5 6 7

  @param binary - the binary image
  @param width - image width
  @param height - image height
  @param connex - 4 or 8 connectivity (4 not implemented)
  @param xret - return for start position x
  @param yret - return for start position y
  @returns the chain code
*/
  char *getchaincode(unsigned char *binary, int width, int height, int connex, int *xret, int *yret)
  {
	  char *answer =0;
	  int *pathx = 0;
	  int *pathy = 0;
	  int N;
	  int dx, dy;
	  int i;
	  char *codes = "3214X0567";
	  int index;

	  assert(connex == 8);
	  N = binary_followboundary(binary, width, height, &pathx, &pathy);
	  if (N < 0)
		  goto error_exit;
	  answer = malloc(N + 1);
	  if (!answer)
		  goto error_exit;
	  for (i = 0; i < N - 1; i++)
	  {
		  dx = pathx[i + 1] - pathx[i];
		  dy = pathy[i + 1] - pathy[i];

		  assert(dx >= -1 && dx <= 1);
		  assert(dy >= -1 && dy <= 1);
		  assert(dy != 0 || dx != 0);

		  index = (dy + 1) * 3 + dx + 1;
		  answer[i] = codes[index];
	  }

	  answer[i] = 0;

	  if (xret)
	  {
		  *xret = N > 0 ? pathx[0] : -1;
	  }
	  if (yret)
	  {
		  *yret = N > 0 ? pathy[0] : -1;
	  }
	  free(pathx);
	  free(pathy);
	  return answer;

  error_exit:
	  free(pathx);
	  free(pathy);
	  free(answer);
	  return 0;
  }

  /**
     Create a perimeter from a  chain code.

	 @param[in, out] binary - the binary image
	 @param width - image width
	 @param height - image height
	 @param[in] code - the chain code
	 @param x - start point x
	 @param y - start point y
	 @returns 0 on success, or the number of pixels out of bounds.
  */
  int chaincodetoperimeter(unsigned char *binary, int width, int height, char *code, int x, int y)
  {
	  int cx, cy;
	  int i;
	  int answer = 0;
	  int index;
	  static int dx[8] = { 1, 1, 0, -1, -1, -1, 0, 1 };
	  static int dy[8] = { 0, -1, -1, -1, 0, 1, 1, 1 };
	  cx = x;
	  cy = y;
	  for (i = 0; code[i]; i++)
	  {
		  if (cx >= 0 && cx < width && cy >= 0 && cy < height)
		  {
			  binary[cy*width + cx] = 1;
		  }
		  else
			  answer++;
		  assert(code[i] >= '0' && code[i] <= '7');
		  index = code[i] - '0';
		  cx += dx[index];
		  cy += dy[index];
	  }

	  return answer;
  }

  typedef struct
  {
	  int x;
	  int y;
  } POINT;

  static int comppoints(const void *e1, const void *e2)
  {
	  const POINT *p1 = e1;
	  const POINT *p2 = e2;

	  if (p1->y != p2->y)
		  return p2->y - p1->y;
	  return p2->x - p1->x;
  }

  /**
     Create a filled shape from a chain code

	 @param[in,out] binary - the binary image
	 @param height - image height
	 @param width  - image width
	 @param[in] code - the chain code
	 @param x - start x point
	 @param y - starty y point;
	 returns 0 on success, 1 partly or all out of bounds, -1 out of memory
  */
  int chaincodetofill(unsigned char *binary, int width, int height, char *code, int x, int y)
  {
	  int N = 0;
	  int i;
	  POINT *points = 0;
	  int sx, ex, ix;
	  int cx, cy;
	  int answer = 0;
	  int index;
	  static int dx[8] = { 1, 1, 0, -1, -1, -1, 0, 1 };
	  static int dy[8] = { 0, -1, -1, -1, 0, 1, 1, 1 };

	  for (i = 0; code[i]; i++)
		  continue;
	  N = i;

	  points = malloc(N * sizeof(POINT));
	  if (!points)
		  goto error_exit;

	  cx = x;
	  cy = y;
	  for (i = 0; code[i]; i++)
	  {
		  if (cx >= 0 && cx < width && cy >= 0 && cy < height)
		  {
			  binary[cy*width + cx] = 1;
		  }
		  else
			  answer++;
		  assert(code[i] >= '0' && code[i] <= '7');
		  index = code[i] - '0';
		  cx += dx[index];
		  cy += dy[index];
	  }

	  qsort(points, N, sizeof(POINT), comppoints);

	  i = 0;
	  while(i < N)
	  {
		  cy = points[i].y;
		  if (cy < 0)
		  {
			  answer = 1;
			  i++;
			  continue;
		  }
		  if (cy >= height)
		  {
			  answer = 1;
			  break;
		  }
		  while (i < N - 1 && points[i].y == cy && points[i + 1].y == cy)
		  {
			  sx = points[i].x;
			  ex = points[i + 1].x;
			  if (sx < 0)
			  {
				  answer = 1;
				  sx = 0;
			  }
			  if (sx >= width)
			  {
				  answer = 1;
				  i += 2;
				  continue;
			  }
			  if (ex >= height)
			  {
				  answer = 1;
				  ex = height - 1;
			  }
			  for (ix = sx; ix <= ex; ix++)
			  {
				  binary[cy * width + ix] = 1;
			  }
			  i += 2;
		  }
	  }

	  free(points); 
	  return answer;

  error_exit:
	  free(points);
	  return -1;
  }

static void getclockwise(int *ncwise, unsigned char *bcwise, unsigned char *neighbours, int b)
{
   static int remap[9][8] =
   {
     {0, 1, 2, 5, 8, 7, 6, 3},
     {1, 2, 5, 8, 7, 6, 3, 0},
     {2, 5, 8, 7, 6, 3, 0, 1},
     {3, 0, 1, 2, 5, 8, 7, 6},
     {4, 4, 4, 4, 4, 4, 4, 4},
     {5, 8, 7, 6, 3, 0, 1, 2},
     {6, 3, 0, 1, 2, 5, 8, 7},
     {7, 6, 3, 0, 1, 2, 5, 8},
     {8, 7, 6, 3, 0, 1, 2, 5},
   };

   int i;

   for(i=0;i<8;i++)
   {
     ncwise[i] = remap[b][i];
     bcwise[i] = neighbours[ncwise[i]];
   }
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
  if(y > 0 && x > 0)        out[0] = binary[(y-1)*width+x-1]; else out[0] = border;
  if(y > 0)                 out[1] = binary[(y-1)*width+x];   else out[1] = border;
  if(y > 0 && x < width-1)  out[2] = binary[(y-1)*width+x+1]; else out[2] = border;
  if(x > 0)                 out[3] = binary[y*width+x-1];     else out[3] = border;
  out[4] = binary[y*width+x];
  if(x < width-1)           out[5] = binary[y*width+x+1];     else out[5] = border;
  if(y < height-1 && x > 0) out[6] = binary[(y+1)*width+x-1]; else out[6] = border;
  if(y < height-1)          out[7] = binary[(y+1)*width+x];   else out[7] = border;
  if(y < height-1 && x < width-1) out[8] = binary[(y+1)*width+x+1]; else out[8] = border; 
}
