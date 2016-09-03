/**@file
   Binaryutils.c

   Source for the binary image processing library simple utlities.

   Put anything that is simple and stands alone more or less here.

   Very much a first, provisional version.

   By Malcolm McLean.

*/
#include <stdlib.h>
#include <string.h>

int morphclose(unsigned char *binary, int width, int height, unsigned char *sel, int swidth, int sheight);
int morphopen(unsigned char *binary, int width, int height, unsigned char *sel, int swidth, int sheight);
int dilate(unsigned char *binary, int width, int height, unsigned char *sel, int swidth, int sheight);
int erode(unsigned char *binary, int width, int height, unsigned char *sel, int swidth, int sheight);
int *labelconnected(unsigned char *binary, int width, int height, int connex, int *Nout);
int eulernumber(unsigned char *binary, int width, int height);
int getbiggestobject(unsigned char *binary, int width, int height, int connex);
int branchpoints(unsigned char *binary, int width, int height, int **xout, int **yout);
int ends(unsigned char *binary, int width, int height, int **xout, int **yout);
unsigned char *perimeter(unsigned char *binary, int width, int height);
void invertbinary(unsigned char *binary, int width, int height);
unsigned char *copybinary(unsigned char *binary, int width, int height);
unsigned char *subbinary(unsigned char *binary, int width, int height, int x, int y, int swidth, int sheight);
void boundingbox(unsigned char *binary, int width, int height, int *x, int *y, int *bbwidth, int *bbheight);
int simplearea(unsigned char *binary, int width, int height);
double complexarea(unsigned char *binary, int width, int height);
void *compressbinary(unsigned char *binary, int width, int height, int *clen);
unsigned char *decompressbinary(unsigned char *comp, int *width, int *height);
int getcontours(unsigned char *binary, int width, int height, double ***x, double ***y, int **Nret);

static int mem_count(unsigned char *pixels, int N, int value);
static void get3x3(unsigned char *out, unsigned char *binary, int width, int height, int x, int y, unsigned char border);

/**
   Morphological close operation.

   @param[in,out] bianry - the binary image
   @param width - image width
   @param height - image height
   @param sel[in] - the selection element
   @param swidth - selection element width
   @param sheight - selection element height
   @returns 0 on sucess, -1 on error.

*/
int morphclose(unsigned char *binary, int width, int height, unsigned char *sel, int swidth, int sheight)
{
  int err;

  err = dilate(binary, width, height, sel, swidth, sheight);
  if(err)
    return err;
  err = erode(binary, width, height, sel, swidth, sheight);
  if(err)
    return err;
  return 0;
}

/**
Morphological open operation.

@param[in,out] bianry - the binary image
@param width - image width
@param height - image height
@param sel[in] - the selection element
@param swidth - selection element width
@param sheight - selection element height
@returns 0 on sucess, -1 on error.

*/
int morphopen(unsigned char *binary, int width, int height, unsigned char *sel, int swidth, int sheight)
{
  int err;

  err = erode(binary, width, height, sel, swidth, sheight);
  if(err)
    return err;
  err = dilate(binary, width, height, sel, swidth, sheight); 
  if(err)
    return err;
  return 0;
}

/**
Dilate operation.

@param[in,out] bianry - the binary image
@param width - image width
@param height - image height
@param sel[in] - the selection element
@param swidth - selection element width
@param sheight - selection element height
@returns 0 on sucess, -1 on error.

*/
int dilate(unsigned char *binary, int width, int height, unsigned char *sel, int swidth, int sheight)
{
	int x, y, sx, sy, ix, iy;
	unsigned char *answer;
    int i;
    int bit;

	answer = malloc(width * height);
	if(!answer)
		return -1;
     
	for(i=0;i<width*height;i++)
      answer[i] = 0;

	for(y=0;y<height;y++)
      for(x=0;x<width;x++)
	  {
		  for(sy=0;sy<sheight;sy++)
		    for(sx =0; sx < swidth; sx++)
			{
		      ix = x + sx - swidth/2;
			  iy = y + sy - sheight/2;
			  if(ix < 0 || ix >= width || sy < 0 || sy >= height)
				  bit = 0;
			  else
		        bit = binary[iy * width + ix];
			  if(bit == 1 && sel[sy*width+sx] == 1)
			  {
                 answer[y*width+x] = 1;
			     break;
			  }
			}
	  }
   for(i=0;i<width*height;i++)
     binary[i] = answer[i];
   free(answer);

   return 0;


}

/**
Erode operation.

@param[in,out] bianry - the binary image
@param width - image width
@param height - image height
@param sel[in] - the selection element
@param swidth - selection element width
@param sheight - selection element height
@returns 0 on sucess, -1 on error.

*/
int erode(unsigned char *binary, int width, int height, unsigned char *sel, int swidth, int sheight)
{
	int x, y, sx, sy, ix, iy;
	unsigned char *answer;
    int i;
	int bit;

	answer = malloc(width * height);
	if(!answer)
		return -1;
	for(i=0;i<width*height;i++)
      answer[i] =1;


	for(y=0;y<height;y++)
      for(x=0;x<width;x++)
	  {
		  for(sy=0;sy<sheight;sy++)
		    for(sx =0; sx < swidth; sx++)
			{
		      ix = x + sx - swidth/2;
			  iy = y + sy - sheight/2;
			  if(ix < 0 || ix >= width || sy < 0 || sy >= height)
				  bit = 0;
			  else
		        bit = binary[iy * width + ix];
			  if(bit == 0 && sel[sy*width+sx] == 1)
			  {
                 answer[y*width+x] = 0;
			     break;
			  }
			}
	  }
   for(i=0;i<width*height;i++)
     binary[i] = answer[i];
   free(answer);

   return 0;
}

/**
  Label connected components in binary image.
  @param[in]  binary - the binary image
  @param      width - image width
  @param      height - image height
  @param      connex - 4 or 8 connectivity
  @param[out] Nout - number of components found
  returns  Width * height array of labels for connected components.
*/
int *labelconnected(unsigned char *binary, int width, int height, int connex, int *Nout)
{
  int *answer;
  int *parents;
  int label;
  int x, y;
  int i;
  int Nlabels = 0;
  int northwest, north, northeast;
  int low, high;
  int ancestor;
  int ancestor2;
  int maxval;
  int N;

  answer = malloc(width*height*sizeof(int));
  parents = malloc(width*height*sizeof(int));
  if(!answer || !parents)
  {
    free(answer);
	free(parents);
	return 0;
  }

  parents[0] = 0;
  for(y=0;y<height;y++)
    for(x=0;x<width;x++)
	{
	  if(binary[y*width+x] == 0)
	    answer[y*width+x] = 0;
	  else
	  {
	    if(x > 0 && binary[y*width+x-1])
          label = answer[y*width+x-1];
		else if(x > 0 && y > 0 && connex == 8 && binary[(y-1)*width+x-1])
		  label = answer[(y-1)*width+x-1];
		else if(y > 0 && binary[(y-1)*width+x])
		  label = answer[(y-1)*width+x];
		else if(y > 0 && x < width-1 && connex == 8 && binary[(y-1)*width+x+1])
		  label = answer[(y-1)*width+x+1];
		else
		{
		  label = ++Nlabels;
		  parents[Nlabels] = Nlabels;
		}
		answer[y*width+x] = label;
		
		northwest = (y > 0 && x > 0) ? answer[(y-1)*width+x-1] : 0;
		north = (y > 0) ? answer[(y-1)*width+x] : 0;
		northeast = (y > 0 && x < width-1) ? answer[(y-1)*width+x+1] : 0;
		if(connex == 8 && northwest != 0 && northwest != label)
		{
	      low = label < northwest ? label : northwest;
		  high = label > northwest ? label : northwest;
		  ancestor = high;
		  while(parents[ancestor] != ancestor)
		    ancestor = parents[ancestor];
		  ancestor2 = low;
          while(parents[ancestor2] != ancestor2)
		    ancestor2 = parents[ancestor2];
		  parents[ancestor] = parents[ancestor2];
		  parents[low] = ancestor2;
		  parents[high] = ancestor2;
		}
		if(north != 0 && north != label)
		{
		  low = label < north ? label : north;
		  high = label > north ? label : north;
		  ancestor = high;
		  while(parents[ancestor] != ancestor)
		    ancestor = parents[ancestor];

		  ancestor2 = low;
          while(parents[ancestor2] != ancestor2)
		    ancestor2 = parents[ancestor2];
		  parents[ancestor] = ancestor2;
		  parents[low] = ancestor2;
		  parents[high] = ancestor2;
		}
		if(connex == 8 && northeast != 0 && northeast != label)
		{
	      low = label < northeast ? label : northeast;
		  high = label > northeast ? label : northeast;
		  ancestor = high;
		  while(parents[ancestor] != ancestor)
		    ancestor = parents[ancestor];
		  ancestor2 = low;
          while(parents[ancestor2] != ancestor2)
		    ancestor2 = parents[ancestor2];
		  parents[ancestor] = ancestor2;
		  parents[low] = ancestor2;
		  parents[high] = ancestor2;
		}
	  }
	}

  maxval  = 0;
  for(i=0;i<width*height;i++)
  {
    if(answer[i])
	{
	  ancestor = answer[i];
	  while(parents[ancestor] != ancestor)
	    ancestor = parents[ancestor];
	  parents[answer[i]] = ancestor;
	  answer[i] = ancestor;
      if(maxval < answer[i])
          maxval = answer[i];
	}
  }

  for(i=0;i<maxval+1;i++)
    parents[i] = -1;
  parents[0] = 0;
  
  N = 1;
  for(i=0;i<width*height;i++)
  {
    if(parents[answer[i]] == -1)
      parents[answer[i]] = N++;
	answer[i] = parents[answer[i]];
  }
  
  free(parents);
  *Nout = N;

  return answer;
}

/**
  Calculate the Euler number for a binary image.

  @params[in] binary - the binary image
  @param      width - image width
  @param      height - image height
  @returns    Euler number, = number of objects - number of holes.
  \note on error returns INT_MIN.
*/
int eulernumber(unsigned char *binary, int width, int height)
{
  int *labels = 0;
  int Nlabels;
  int *bglabels = 0;
  int Nbglabels;
  int i;
  unsigned char *edgeflags = 0;
  int answer;
   
  labels = labelconnected(binary, width, height, 8, &Nlabels);
  if(!labels)
	  goto error_exit;
  free(labels);
  labels = 0;
  invertbinary(binary, width, height);
  bglabels = labelconnected(binary, width, height, 4, &Nbglabels);
  if(!bglabels)
    goto error_exit;
  invertbinary(binary, width, height);
  edgeflags = malloc(Nbglabels);
  if(!edgeflags)
	  goto error_exit;
  memset(edgeflags, 1, Nbglabels);
  edgeflags[0] = 0;
  /* if a hole is on the edge it is not a hole */
  for(i=0;i<width;i++)
  {
    edgeflags[bglabels[i]] = 0;
    edgeflags[bglabels[(height-1)*width+i]] = 0;
  }
  for(i=1;i<height-1;i++)
  {
    edgeflags[bglabels[i*width]] = 0;
	edgeflags[bglabels[i*width+width-1]] = 0;
  }
  free(bglabels);
  answer = Nlabels -1;
  for(i=0;i<Nbglabels;i++)
    if(edgeflags[i])
	  answer--;
  return answer;
error_exit:
  free(labels);
  free(bglabels);
  free(edgeflags);
  return (int) ~(((unsigned)~0) >> 1);
}

/**
   Erase everything except biggest object in bianry image.

   @param [in.out] binary - the binary image
   @param width - image width
   @param height - image height
   @param connex - 4 or 8 connectivity
   @returns 0 on success, -1 on fail
*/
int getbiggestobject(unsigned char *binary, int width, int height, int connex)
{
  int *labels;
  int Nlabels;
  int *hist = 0;
  int i;
  int best = 0;
  int bestlabel = -1;

  labels = labelconnected(binary, width, height, connex, &Nlabels);
  if(!labels)
    goto error_exit;
  hist = malloc(Nlabels * sizeof(int));
  if(!hist)
    goto error_exit;
  for(i=0;i<Nlabels;i++)
    hist[i] = 0;
  for(i=0;i<width*height;i++)
    hist[labels[i]]++;
  for(i=1;i<Nlabels;i++)
    if(hist[i] > best)
	{
	  best = hist[i];
	  bestlabel = i;
	}
  for(i=0;i<width*height;i++)
    if(labels[i] == bestlabel)
	  binary[i] = 1;
	else
	  binary[i] = 0;
  free(labels);
  free(hist);
  return 0;
error_exit:
  free(labels);
  free(hist);
  return -1;
}

/**
   Get branch points

   @param[in]    binary - the binary image
   @param        width - image width
   @param        height - image height
   @param[out]   xout - return for branch point x-coordiantes (malloced)
   @param[out]   yout - return for branch point y-coordinates ( malloced)
   @returns Number of branch pints found, -1 on fail.
*/
int branchpoints(unsigned char *binary, int width, int height, int **xout, int **yout)
{
  int loopindex[9] = {0, 1, 2, 5, 8, 7, 6, 3, 0};
  unsigned char neighbours[9];
  unsigned char loop[9];
  int crossings;
  int x, y;
  int i;
  void *temp;
  int *branchx = 0;
  int *branchy = 0;
  int answer = 0;

  for(y=0;y<height;y++)
	for(x=0;x<width;x++)
	{
      if(binary[y*width+x])
	  {
	    get3x3(neighbours, binary, width, height, x, y, 0);
		for(i=0;i<9;i++)
		  loop[i] = neighbours[loopindex[i]];
		crossings = 0;
		for(i=1;i<9;i++)
		  if(loop[i] != loop[i-1])
			  crossings++;
		if(crossings > 4)
		{
		  temp = realloc(branchx, (answer +1) * sizeof(int));
		  if(!temp)
		    goto error_exit;
		  branchx = temp;

		  temp = realloc(branchy, (answer +1) * sizeof(int));
		  if(!temp)
		    goto error_exit;
		  branchy = temp;

		  branchx[answer] = x;
          branchy[answer] = y;
		  answer++;
		}
	  }
	}
	*xout = branchx;
	*yout = branchy;
	return answer;
error_exit:
	*xout = 0;
	*yout = 0;
	free(branchx);
	free(branchy);
	return -1;
}

/**
Get line end points

@param[in]      binary - the binary image
@param          width - image width
@param          height - image height
@param[out]     xout - return for branch point x-coordiantes (malloced)
@param[out]     yout - return for branch point y-coordinates ( malloced)
@returns Number of line end found, -1 on fail.
*/
int lineends(unsigned char *binary, int width, int height, int **xout, int **yout)
{
    int loopindex[9] = {0, 1, 2, 5, 8, 7, 6, 3, 0};
    unsigned char neighbours[9];
    unsigned char loop[9];
    int crossings;
    int x, y;
    int i;
    void *temp;
    int *branchx = 0;
    int *branchy = 0;
    int answer = 0;
    int Nneighbours;
    
    for(y=0;y<height;y++)
        for(x=0;x<width;x++)
        {
            if(binary[y*width+x])
            {
                get3x3(neighbours, binary, width, height, x, y, 0);
                Nneighbours = mem_count(neighbours, 9, 1);
                if(Nneighbours != 2 && Nneighbours != 3)
                    continue;
                for(i=0;i<9;i++)
                    loop[i] = neighbours[loopindex[i]];
                crossings = 0;
                for(i=1;i<9;i++)
                    if(loop[i] != loop[i-1])
                        crossings++;
                if(crossings == 2)
                {
                    temp = realloc(branchx, (answer +1) * sizeof(int));
                    if(!temp)
                        goto error_exit;
                    branchx = temp;
                    
                    temp = realloc(branchy, (answer +1) * sizeof(int));
                    if(!temp)
                        goto error_exit;
                    branchy = temp;
                    
                    branchx[answer] = x;
                    branchy[answer] = y;
                    answer++;
                }
            }
        }
    *xout = branchx;
    *yout = branchy;
    return answer;
error_exit:
    *xout = 0;
    *yout = 0;
    free(branchx);
    free(branchy);
    return -1;

}

/**
Get ends

@param[in]    binary - the binary image
@param        width - image width
@param        height - image height
@param[out]   xout - return for end x-coordiantes (malloced)
@param[out]   yout - return for end y-coordinates ( malloced)
@returns Number of branch pints found, -1 on fail.
\note naive algorithm, just returns everythign with 1 neighbout
*/
int ends(unsigned char *binary, int width, int height, int **xout, int **yout)
{
  int *endx = 0;
  int *endy = 0;
  int answer = 0;
  void *temp;
  unsigned char neighbours[9];
  int Nneighbours;
  int x, y;
    
  for(y=0;y<height;y++)
    for(x=0;x<width;x++)
	{
	  if(binary[y*width+x])
	  {
	    get3x3(neighbours, binary, width, height, x, y, 0);
		Nneighbours = mem_count(neighbours, 9, 1);
		if(Nneighbours == 2)
		{
		  temp = realloc(endx, (answer+1) * sizeof(int));
		  if(!temp)
		    goto error_exit;
		  endx = temp;
		  temp = realloc(endy, (answer+1) * sizeof(int));
		  if(!temp)
			goto error_exit;
          endy = temp;
          endx[answer] = x;
		  endy[answer] = y;
		  answer++;
		}
	  }
	}
  *xout = endx;
  *yout = endy;
  return answer;

error_exit:
  free(endx);
  free(endy);
  *xout = 0;
  *yout = 0;
  return -1;
}

/**
  get perimeter pixels.

  @param[in] binary - the bianry image
  @param width - image width
  @param height - image height
  @returns The perimeter pixels.
*/
unsigned char *perimeter(unsigned char *binary, int width, int height)
{
  unsigned char *answer;
  unsigned char neighbours[9];
  int Nneighbours;
  int x, y;
  

  answer = malloc(width * height);
  if(!answer)
    return 0;
  for(y=0;y<height;y++)
    for(x=0;x<width;x++)
    {
	  if(binary[y*width+x])
	  {
	    get3x3(neighbours, binary, width, height, x, y, 0);
        Nneighbours = mem_count(neighbours, 9, 1);
		if(Nneighbours == 9)
	      answer[y*width+x] = 0;
		else
		  answer[y*width+x] = 1;
	  }
	  else
		answer[y*width+x] = 0;

    }

  return answer;
}

/**
  Invert a binary image.
  @param[in,out] binary - the binary image
  @param width - image width
  @param height - image height
  \note in place.
*/
void invertbinary(unsigned char *binary, int width, int height)
{
  int i;

  for(i=0;i<width*height;i++)
    binary[i] = 1 - binary[i];
}

/**
  Copy a binary image.

  @param[in] binary - the binary image
  @param width - iamge width
  @param height - image height
  @returns Pointer to malloced copy of image, 0 on out of memory.
*/
unsigned char *copybinary(unsigned char *binary, int width, int height)
{
  unsigned char *answer;

  answer = malloc(width * height);
  if(!answer)
    return 0;
  memcpy(answer, binary, width * height);

  return answer;
}

/**
  Add a border to binary image (useful for removed edge conditions)
  @param[in]  binary - the binary image
  @param width - image width
  @param height - image height
  @returns Pointer to malloced enlarged image, 0 on out of memory.
*/
unsigned char *binary_addborder(unsigned char *binary, int width, int height, int border, unsigned char fill)
{
	unsigned char *answer;
	int dwidth, dheight;
	int x, y;

	dwidth = width + 2 * border;
	dheight = height + 2 * border;
	answer = malloc(dwidth * dheight);
	if (!answer)
		goto error_exit;


	for (y = 0; y < border; y++)
	{
		for (x = 0; x < dwidth; x++)
		{
			answer[y*dwidth + x] = fill;
		}
	}

	for (y = border; y < dheight - border; y++)
	{
		for (x = 0; x < border; x++)
			answer[y*dwidth + x] = fill;
		for (x = border; x < dwidth-border; x++)
			answer[y*dwidth + x ] = binary[(y - border)*width + x - border];
		for (x = dwidth - border; x < dwidth; x++)
			answer[y*dwidth + x] = fill;
	}
	for (y = dheight - border; y < dheight; y++)
	{
		for (x = 0; x < dwidth; x++)
		{
			answer[y*dwidth + x] = fill;
		}
	}

	return answer;

	error_exit:
	   return 0;
}

/**
  Add border with wrapping (useful for some types of filters).

  @param[in] binary - the binary image
  @param width - image width
  @param height - image height
  @param border - number of border pixels to add
  @returns The enlarged image, 0 on out of memory.
*/
unsigned char *binary_addborderwrapped(unsigned char *binary, int width, int height, int border)
{
	unsigned char *answer;
	int dwidth, dheight;
	int x, y;
	int sx, sy;

	answer = binary_addborder(binary, width, height, border, 0);
	if (!answer)
		goto error_exit;

	dwidth = width + 2 * border;
	dheight = height + 2 * border;

	for (y = 0; y < border; y++)
	{
		sy = height - border + y;
		while (sy < 0)
			sy += height;

		for (x = 0; x < border; x++)
		{
			sx = width - border + x;
			while (sx < 0)
				sx += width;
			answer[y*dwidth + x] = binary[sy*width + sx];
		}

		for (x = border; x < dwidth-border; x++)
		{
			sx = x - border;
			answer[y*dwidth + x] = binary[sy*width + sx];
		}

		for (x = dwidth - border; x < dwidth; x++)
		{
			sx = x - dwidth + border;
			while (sx >= width)
				sx -= width;
			answer[y*dwidth + x] = binary[sy*width + sx];
		}
	}

	for (y = border; y < dheight - border; y++)
	{
		sy = y - border;

		for(x = 0; x < border; x++)
		{
			sx = width - border + x;
			while (sx < 0)
				sx += width;
			answer[y*dwidth + x] = binary[sy*width + sx];
		}

		for (x = dwidth - border; x < dwidth; x++)
		{
			sx = x - dwidth + border;
			while (sx >= width)
				sx -= width;
			answer[y*dwidth + x] = binary[sy*width + sx];
		}
	}

	for (y = dheight - border; y < dheight; y++)
	{
		sy = y - dheight + border;
		while (sy >= height)
			sy -= height;

		for (x = 0; x < border; x++)
		{
			sx = width - border + x;
			while (sx < 0)
				sx += width;
			answer[y*dwidth + x] = binary[sy*width + sx];
		}

		for (x = border; x < dwidth - border; x++)
		{
			sx = x - border;
			answer[y*dwidth + x] = binary[sy*width + sx];
		}

		for (x = dwidth - border; x < dwidth; x++)
		{
			sx = x - dwidth + border;
			while (sx >= width)
				sx -= width;
			answer[y*dwidth + x] = binary[sy*width + sx];
		}
	}

	return answer;
error_exit:
	free(answer);
	return 0;

}

/**
   Remove a border from a binary image.

   @param[in]  binary - the binary image
   @param width - image width
   @param height - image height
   @param border - number of border pixels to remove
   @returns Malloced destination image,  0 on failure.
*/
unsigned char *binary_removeborder(unsigned char *binary, int width, int height, int border)
{
	int dwidth, dheight;
	unsigned char *answer = 0;
	int x, y;

	dwidth = width - 2 * border;
	dheight = height - 2 * border;
	if (dwidth < 0 || dheight < 0)
		goto error_exit;
	answer = malloc(dwidth * dheight);
	if (!answer)
		goto error_exit;
	for (y = 0; y < dheight; y++)
	{
		for (x = 0; x < dwidth; x++)
		{
			answer[y*dwidth + x] = binary[(y + border)*width + x + border];
		}
	}
	return answer;
error_exit:
	free(answer);
	return 0;
}

/**
  Take a sub-image of a binary image.

  @param[in] binary - the binary image
  @param width - image width
  @param height - image height
  @param x - top left x co-ordinate of sub-image
  @param y - top left y co-ordinate of sub-image
  @param swidth - sub image width
  @param sheight - sub image height
  @returns Malloced pointer to sub image.
  @note If the sub-image extends the image boundary, it is zero padded, so x and y can be negative.
*/
unsigned char *subbinary(unsigned char *binary, int width, int height, int x, int y, int swidth, int sheight)
{
  unsigned char *answer;
  int ix, iy;

  answer = malloc(swidth * sheight);
  if(!answer)
    return 0;

  for(iy=y;iy<y+sheight;iy++)
  {
    if(y <0 || y > height -1)
	{
	  for(ix=0;ix<swidth;ix++)
	    answer[(iy-y)*swidth+ix] = 0;
	}
	else
	{
	  for(ix=x;ix<0;ix++)
	    answer[(iy-y)*swidth + ix - x] = 0;
	  for(ix=0;ix<swidth&&ix<width;ix++)
	    answer[(iy-y)*swidth + ix] = binary[iy*width+ix+x];
	  for(ix = width;ix<swidth;ix++)
        answer[(iy-y)*swidth + ix] = 0;
	}
  }

  return answer;
}

/**
  Get the bounding box of the set pixels in binary image.

  @param[in] binary - the binary image
  @param     width - image width
  @param     height - image height
  @param[out] x return for bounding box top left x co-ordinate
  @param[out] y return for bounding box top left y co-ordinate
  @param[out] bbwidth - return for bounding box width
  @param[out] bbheight - return for bounding box height
  @note x and y are -1 if there are no set pixels in the image.

*/
void boundingbox(unsigned char *binary, int width, int height, int *x, int *y, int *bbwidth, int *bbheight)
{
  int i, ii;

  for(i=0;i<width*height;i++)
    if(binary[i])
	  break;
  if(i == width * height)
  {
    *x = -1;
	*y = -1;
	*bbwidth = 0;
	*bbheight = 0;
	return;
  }
  *y = i/width;
  for(i=width*height-1;i>=0;i--)
    if(binary[i])
	  break;
  *bbheight = i/width - *y + 1;

  for(i=0;i<width;i++)
  {
    for(ii=*y;ii<*y+*bbheight;ii++)
      if(binary[ii*width+i])
	    break;
	if(ii != *y + *bbheight)
	{
	  *x = i;
	  break;
	}
  }

  for(i=width-1;i>=0;i--)
  {
    for(ii=*y;ii<*y+*bbheight;ii++)
      if(binary[ii*width+i])
	    break;
	if(ii != *y + *bbheight)
	{
	  *bbwidth = i - *x + 1;
	  break;
	}
  }
}

/**
  Get the area (number of set pixels) in a binary image.

  @param[in] binary - the binary image
  @param width - image width
  @param height - image height
  @returns number of set pixels.
*/
int simplearea(unsigned char *binary, int width, int height)
{
  int i;
  int answer = 0;

  for(i=0;i<width*height;i++)
    if(binary[i])
      answer++;

  return answer;
}

/**
  Get the area of binary image, weighting pixels by pattern.
  @param[in] binary - the binary image
  @param width - image width
  @param height - image height
  @returns weighted area of the image.
*/
double complexarea(unsigned char *binary, int width, int height)
{
  int x, y;
  double answer = 0;
  int p1, p2, p3, p4;

  for(y=-1;y<height;y++)
    for(x=-1;x<width;x++)
	{
	  if(x >= 0 && y >= 0) p1 = binary[y*width+x]; else p1 = 0;
	  if(x < width -1 && y >= 0) p2 = binary[y*width+x+1]; else p2 = 0;
	  if(x >= 0 && y < height-1) p3 = binary[(y+1)*width+x]; else p3 = 0;
	  if(x < width-1 && y < height -1) p4 = binary[(y+1)*width+x+1]; else p4 = 0;

	  switch(p1+p2+p3+p4)
	  {
		case 0: answer += 0; break;
		case 1: answer += 3; break; 
		case 2: if( (p1 && p3) || (p2 && p4))
		          answer += 6;
				else
				  answer += 4;
			break;
		case 3: answer += 7; break;
		case 4: answer += 8; break;
	  }
	}

  return answer / 8.0;

}

/**
  Quick run-length compression of a binary image.
  @param[in]  binary - the binary image
  @param      width - image width
  @param      height - image height
  @param[out] clen - return pointer for length of compressed data
  @returns pointer to malloced compressed data.
*/
void *compressbinary(unsigned char *binary, int width, int height, int *clen)
{
  int i, j;
  unsigned char *answer;
  int flag;
  int Nwritten = 0;
  int runlen = 0;

  *clen = -1;
  if(width * height == 0)
  {
    answer = malloc(6);
	if(!answer)
		return 0;
	for(i=0;i<6;i++)
	  answer[i]  = 0;
	*clen = 6;
	return answer;
  }

  flag = binary[0];
  Nwritten = 0;
  for(i=0;i<width*height;i++)
  {
    if(binary[i] == flag)
	{
      runlen++;
	  if(runlen == 256)
	  {
        runlen = 1;
		Nwritten += 2;
	  }
	}
	else
	{
	  runlen = 1;
	  Nwritten++;
	  flag = 1 - flag;
	}
  }
  
  answer = malloc(Nwritten + 6);
  if(!answer)
	  return 0;
  
  *clen = Nwritten + 6;

  answer[0] = (width >> 8) & 0xFF;
  answer[1] = width & 0xFF;
  answer[2] = (height >> 8) & 0xFF;
  answer[3] = height & 0xFF;
  answer[4] = binary[0];
  j = 5;

  runlen = 0;
  flag = binary[0];
  for(i=0;i<width*height;i++)
  {
    if(binary[i] == flag)
	{
	  runlen++;
	  if(runlen == 256)
	  {
	    answer[j++] = 255;
		answer[j++] = 0;
		runlen = 1;
	  }
	}
	else
	{
	  answer[j++] = runlen;
	  runlen = 1;
	  flag = 1 - flag;
	}
  }
  answer[j++] = runlen;
  
  return answer;
}

/**
  Decompression of run-length compressed binary image.
  @param[in] comp - pointer to the compressed data
  @param[out] width - return pointer for image width
  @param[out] height - return pointer for image height
  @returns decompressed image data
*/
unsigned char *decompressbinary(unsigned char *comp, int *width, int *height)
{
  unsigned char *answer;
  int w, h;
  int flag;
  int i, j;
  int runlen;

  *width = -1;
  *height = -1;
  w = ( (int) comp[0] << 8) | comp[1];
  h = ( (int) comp[2] << 8) | comp[3];
  flag = comp[4];

  answer = malloc(w * h);
  if(!answer)
	  return 0;
  i = 0;
  j = 5;
  while(i < w * h)
  {
    runlen = comp[j++];
	while(runlen--)
	  answer[i++] = flag;
    flag = 1 - flag;
  }
  *width = w;
  *height = h;
 
  return answer;
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

  for(i=0;i<N;i++)
    if(pixels[i] == value)
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

static void get2x2(unsigned char *out, unsigned char *binary, int width, int height, int x,
                   int y, unsigned char border)
{
    if(x <= 0 || y <= 0)
        out[0] = border;
    else
        out[0] = binary[(y-1)*width+x-1];
    if(y <= 0 || x >= width)
        out[1] = border;
    else
        out[1] = binary[(y-1)*width+x];
    if(y >= height || x <= 0)
        out[2] = border;
    else
        out[2] = binary[y*width+x-1];
    if(x >= width || y >= height)
        out[3] = border;
    else
        out[3] = binary[y*width+x];
}

#define CNT_TOPLEFT 0x80
#define CNT_TOPRIGHT 0x40
#define CNT_BOTTOMRIGHT 0x20
#define CNT_BOTTOMLEFT 0x10

#define CNT_RIGHT 1
#define CNT_DOWN 2
#define CNT_LEFT 3
#define CNT_UP 4

static int walkcontour(unsigned char *binary, int width, int height, int x, int y, int dir, double **cx,
                        double **cy, int *Nret);


/**
   Go round a binary image, extracting the contours of the set pixels.
   
   @param[in]    binary - the image
   @param width - image width
   @param height - image height
   @param[out] x -  return for x co-ordinates of contours (malloced)
   @param[out] y -  return for y co-ordinates of contours (malloced)
   @param[out] Nret - return for contour lengths (malloced)
   @returns Number contours found, -1 on out of memory.
 */
int getcontours(unsigned char *binary, int width, int height, double ***x, double ***y, int **Nret)
{
    int xi, yi;
    int i;
    unsigned char neighbours[9];
    int N;
    double *cx, *cy;
    int cN;
    void *temp;
    double **contourx = 0;
    double **contoury = 0;
    int *contourN = 0;
    int err;
    int answer = 0;
    int capacity = 0;
    
    for(yi=0;yi<height;yi++)
    {
        for(xi=0;xi<width;xi++)
        {
            if(binary[yi*width + xi] & 0x01)
            {
                get2x2(neighbours, binary, width, height, xi, yi, 0);
                N = 0;
                for(i=0;i<4;i++)
                    N += neighbours[i] & 0x01;
                if(N != 4)
                {
                    if( (binary[yi*width+xi] & CNT_TOPLEFT) == 0 &&
                       (neighbours[0] & neighbours[1] & neighbours[3] &0x01) == 0)
                    {
                       err =  walkcontour(binary, width, height, xi, yi, CNT_RIGHT, &cx, &cy, &cN);
                       if(err == 0)
                       {
                           if(answer >= capacity)
                           {
                               capacity = capacity + capacity/2 + 10;
                               temp = realloc(contourN, capacity * sizeof(int));
                               contourN = temp;
                                temp = realloc(contourx, capacity * sizeof(double *));
                               contourx = temp;
                               temp = realloc(contoury, capacity * sizeof(double *));
                               contoury = temp;
                           }
                           contourN[answer] = cN;
                           contourx[answer] = cx;
                           contoury[answer] = cy;
                           answer++;
                       }
                    }
                    /*
                    if( (binary[yi*width+xi] & CNT_TOPRIGHT) == 0 &&
                       (neighbours[1] & neighbours[2] & neighbours[5] &0x01) == 0)
                    {
                        walkcontour(binary, width, height, xi +1, yi, CNT_DOWN, &cx, &cy, &cN);
 
                    }
                    if( (binary[yi*width+xi] & CNT_BOTTOMRIGHT) == 0 &&
                       (neighbours[5] & neighbours[7] & neighbours[8] &0x01) == 0)
                    {
                        walkcontour(binary, width, height, xi, yi+1, CNT_LEFT, &cx, &cy, &cN);
                        
                    }
                    if( (binary[yi*width+xi] & CNT_BOTTOMLEFT) == 0)
                    {
                        if( (binary[yi*width+xi] & CNT_BOTTOMRIGHT) == 0 &&
                           (neighbours[5] & neighbours[7] & neighbours[8] &0x01) == 0)

                        walkcontour(binary, width, height, xi+1, yi+1, CNT_UP, &cx, &cy, &cN);
                        
                    }
                     */

                }
            }
        }
    }
    
    for(yi=0;yi<height;yi++)
    {
        for(xi=0;xi<width;xi++)
            binary[yi*width+xi] &= 0x01;
    }
    *x = contourx;
    *y = contoury;
    *Nret = contourN;
    
    return answer;
}
    
static int walkcontour(unsigned char *binary, int width, int height, int x, int y, int dir, double **cx,
                 double **cy, int *Nret)
{
    int N = 0;
    int advance;
    int capacity = width + 3;
    double *xwalk, *ywalk;
    double *temp;
    unsigned char neighbours[9];
    int xi, yi;
    int mask;
    
    switch(dir)
    {
        case CNT_RIGHT: mask = CNT_TOPLEFT; break;
        case CNT_DOWN: mask = CNT_TOPRIGHT; break;
        case CNT_LEFT: mask = CNT_BOTTOMRIGHT; break;
        case CNT_UP: mask = CNT_BOTTOMLEFT; break;
        
    }
    mask = CNT_TOPLEFT;
    
    xwalk = malloc(capacity * sizeof(double));
    ywalk = malloc(capacity * sizeof(double));
    if(!xwalk || ! ywalk)
        goto out_of_memory;
    xi = x;
    yi = y;
    while(1)
    {
        get2x2(neighbours, binary, width, height, xi, yi, 0);
        advance = 0;
        while(advance == 0)
        {
            switch(dir)
            {
                case CNT_RIGHT:
                    if(neighbours[1] & 0x01)
                        dir = CNT_UP;
                    else
                        advance = 1;
                    break;
                case CNT_DOWN:
                    if(neighbours[3] & 0x01)
                        dir = CNT_RIGHT;
                    else
                        advance = 1;
                    break;
                case CNT_LEFT:
                    if(neighbours[2] & 0x01)
                        dir = CNT_DOWN;
                    else
                        advance = 1;
                    break;
                case CNT_UP:
                    if(neighbours[0] & 0x01)
                        dir = CNT_LEFT;
                    else
                        advance = 1;
                    break;
                
            }
        }
        
        xwalk[N] = xi;
        ywalk[N] = yi;
        N++;
        if(N >= capacity)
        {
            capacity += capacity/2;
            
            temp = realloc(xwalk, capacity * sizeof(double) );
            if(!temp)
                goto out_of_memory;
            xwalk = temp;
            
            temp = realloc(ywalk, capacity * sizeof(double) );
            if(!temp)
                goto out_of_memory;
            ywalk = temp;
        }
        advance = 0;
        
        if(yi < height && xi < width)
            binary[yi*width+xi] |= CNT_TOPLEFT;
        if(yi < height && xi > 0)
            binary[yi*width+xi-1] |= CNT_TOPRIGHT;
        if(yi > 0 && xi < width)
            binary[(yi-1)*width+xi] |= CNT_BOTTOMLEFT;
        if(xi > 0 && yi > 0)
                binary[(yi-1)*width+xi-1] |= CNT_BOTTOMRIGHT;
        
        switch(dir)
        {
            case CNT_RIGHT: xi++; break;
            case CNT_DOWN: yi++; break;
            case CNT_LEFT: xi--; break;
            case CNT_UP: yi--; break;
        }
        
        get2x2(neighbours, binary, width, height, xi, yi, 0);
        while(advance == 0)
        {
            switch(dir)
            {
                case CNT_RIGHT:
                    if( !(neighbours[3] & 0x01))
                        dir = CNT_DOWN;
                    else
                        advance = 1;
                    break;
                case CNT_DOWN:
                    if(!(neighbours[2] & 0x01))
                        dir = CNT_LEFT;
                    else
                        advance = 1;
                    break;
                case CNT_LEFT:
                    if(!(neighbours[0] & 0x01))
                        dir = CNT_UP;
                    else
                        advance = 1;
                    break;
                case CNT_UP:
                    if(!(neighbours[1] & 0x01))
                        dir = CNT_RIGHT;
                    else
                        advance = 1;
                    break;
                    
            }
        }
        if(yi == y && xi == x && (binary[yi*width+xi] & mask) )
            break;
        
    }
    *cx = xwalk;
    *cy = ywalk;
    *Nret = N;
    return 0;
out_of_memory:
    free(xwalk);
    free(ywalk);
    *cx = 0;
    *cy  =0;
    *Nret  = 0;
    return -1;
    
}
