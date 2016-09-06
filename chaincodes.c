#include <stdlib.h>

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

  do
  {
    if(bufflen < (N+1) *2)
    {
      temp  = realloc(buffer, bufflen * 2 + 64);
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
    for(i=0;i<N;i++)
      (*pathx)[i] = buffer[i*2];
  }
  if(pathy)
  {
    *pathy = malloc(N * sizeof(int));
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
