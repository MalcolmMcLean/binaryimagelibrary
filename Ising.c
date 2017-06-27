#include <stdlib.h>
#include <math.h>

#include "Ising.h"

/* CRITICAL TEMP = 2.269; */


static int getNsetneighbours(unsigned char *binary, int width, int height,
                             int x, int y);

/**
   @brief Step a simple Ising model
   @param binary - the binary image
   @param width - image width
   @param height - image height
   @param t - temperature (2.269 = critical)
   
   Step an Isng model. It's just a simple model with all connections
   the same strenght and no external field. For binary image processsing
   pruposes try passing a low temperature from random.
  
 */
void  isingstep(unsigned char *binary, int width, int height, double t)
{
   int x, y;
   int Nset;
   double p;
   double prob8 = exp(-8.0/t);
   double prob4 = exp(-4.0/t);
   double ss0, ssn;
   int i;

  
   for(i=0;i<width * height;i++)
   {  
      x = rand() %  width;
      y = rand() %  height;
    
      p = rand()/(RAND_MAX + 1.0); 
      Nset = getNsetneighbours(binary, width, height, x, y);

      if(Nset > 2)
      {
        if( binary[y*width+x] == 0)
        {
            binary[y*width+x] = 1;
        }
        else if((Nset == 3 && p < prob4) || (Nset == 4 && p < prob8))
        {
            binary[y*width+x] = 0;
        }
      }
      else if(Nset == 2)
      {
         binary[y*width+x] = (p < 0.5) ? 0 : 1;
      }   
      else if(Nset < 2)
      {
        if(binary[y*width+x] == 1)
        {
            binary[y*width+x] = 0;
        }
        else if((Nset == 1 && p < prob4) || (Nset == 0 && p < prob8))
        {
           binary[y*width+x] = 1;
        }
      }
    }
}



static int getNsetneighbours(unsigned char *binary, int width, int height,
			     int x, int y)
{
  int xleft = x ? x - 1 : width -1;
  int xright = (x < width-1) ? x + 1 : 0;
  int yup = y ? y - 1: height -1;
  int ydown = (y < height -1) ? y + 1: 0;

  return binary[y*width+xleft] + 
         binary[y*width+xright] +
         binary[yup * width + x] +
         binary[ydown * width + x];
}
