#include <stdlib.h>
#include <limits.h>

#define min2(a,b) ((a) < (b) ? (a) : (b))
#define min4(a,b,c,d) min2(min2(a,b), min2(c,d))

/**
   @brief distance transform using chamfer 3-4 rule
   @param[in] binary - the binary image
   @param width - image width
   @param height  image height
   @return the transform as integers


   The transform is based on the idea that the distance
   scores like this

   4  3  4
   3  x  .  the ratio 4:3 stands for 1: root2

*/
int *chamfer34(unsigned char *binary, int width, int height)
{
  int *padded = 0;
  int *answer =  0;
  int x, y;
  int i;
  int pwidth = width + 2;
  int distup, distdown;

  padded = malloc( (width +2) * (height+2) * sizeof(int));
  if(!padded)
    goto out_of_memory;
  answer = malloc(width*height*sizeof(int));
  if(!answer)
    goto out_of_memory;

  for(i=0;i<(width+2)*(height+2);i++)
    padded[i] = 0;

  for(y=0;y<height;y++)
    for(x=0;x<width;x++)
      if(binary[y*width+x] != 0)
        padded[(y+1)*pwidth+x+1] = INT_MAX - 10;

  for(y=1;y<height+1;y++)
  {
    for(x=1;x<width+1;x++)
    {
      distup = min4(padded[(y-1)*pwidth+x-1]+4,
                    padded[(y-1)*pwidth+x]+3,
                    padded[(y-1)*pwidth+x+1]+4,
                    padded[y*pwidth+x-1] +3);
      if(distup < padded[y*pwidth+x])
        padded[y * pwidth+x] = distup; 
    }
  }

 
  for(y=height+1; y >= 1; y--)
  {
    for(x=width+1; x >= 1; x--)
    {
      distdown = min4(padded[(y+1)*pwidth+x+1]+4,
                      padded[(y+1)*pwidth+x]+3,
                      padded[(y+1)*pwidth+x-1]+4,
                      padded[y*pwidth+x+1]+3);
      if(distdown < padded[y*pwidth+x])
	padded[y*pwidth+x] = distdown;

    }
  }

  for(y=0;y<height;y++)
    for(x=0;x<width;x++)
      answer[y*width+x] = padded[(y+1)*pwidth+x+1];
    
  free(padded);
  return answer;
 out_of_memory:
  free(padded);
  free(answer);
  return 0;
}

#include <stdio.h>
int testchamfer34main(void)
{
  unsigned char *binary = malloc(20*20);
  int i;
  int *res;
  int ix, iy;

  for(i=0;i<20*20;i++)
    binary[i] = 0;
  for(i=0;i<20*20;i++)
  {
    ix = (i % 20) - 10;
    iy = (i / 20) - 10;
    if(ix*ix + iy*iy < 100)
      binary[i] = 1;
  }

  res = chamfer34(binary, 20, 20);
  for(i=0;i<20*20;i++)
  {
    printf("% 3d ", res[i]);
    if((i % 20) == 19)
      printf("\n");
  }

  return 0;
}
