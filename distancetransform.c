/**@file
 * 
 * Calculates the distance transform of an image using Saito's method
 * 
 * Saito's exact EDT  - a Euclidean distance transfrom that returns
 * exact results.
 *  
 *
 */
#include <stdlib.h>
#include <math.h>

float *euclideandistancetransform(unsigned char *binary, int width, int height);
int *edt_saito(unsigned char *binary, int width, int height);

/*
  gift-wraps edt_saito.
  The function doesn't handle the case where set pixels touch the border
  Also, it returns the square of the distnace as an integer

  @param[in] binary - the binary image
  @param width - image width
  @param height - image height
  @returns The distance transform as a float.
*/
float *euclideandistancetransform(unsigned char *binary, int width, int height)
{
  unsigned char *bordered = 0;
  int *dt = 0;
  float *answer = 0;
  int x, y;

  bordered = malloc( (width + 2) * (height + 2) );
  if(!bordered)
	goto error_exit;
   
  for(y=0;y<height;y++)
    for(x=0;x<width;x++)
	  bordered[ (y + 1)* (width + 2) + x + 1] = binary[y*width+x];
	  
  for(x=0;x<width+2;x++)
  {
    bordered[x] = 0;
    bordered[(height+1)*(width+2)+x] = 0;
  }
  for(y=0;y<height+2;y++)
  {
    bordered[y*(width+2)] = 0;
	bordered[y*(width+2)+width+1] = 0;
  }
  
  dt = edt_saito(bordered, width + 2, height + 2);
  if(!dt)
	goto error_exit;

  answer = malloc(width * height * sizeof(float));
  if(!answer)
	  goto error_exit;

  for(y=0;y<height;y++)
    for(x=0;x<width;x++)
	  answer[y*width+x] = (float) sqrt(dt[(y+1)*(width+2) + x + 1]);

  free(dt);
  free(bordered);
  return answer;
error_exit:
  free(bordered);
  free(dt);
  free(answer);
  return 0;
}

/**
 * Distance transform.
 *
 * @param[in] binary - the binary image
 * @param width - image width
 * @param height - image height
 * @returns Square of Euclidean distance transform.
 *
 * Final implementation by R. Fabbri, slightly adjusted by Malcolm McLean
 * based on two independent implementations by O. Cuisenaire
 * and J. C. Torelli.
 *
 * PAPER
 *    T. Saito and J.I. Toriwaki, "New algorithms for Euclidean distance 
 *    transformations of an n-dimensional digitised picture with applications",
 *    Pattern Recognition, 27(11), pp. 1551-1565, 1994
 *
 * A nice overview of Saito's method may be found at:
 *    Chapter 2 of "Distance transformations: fast algorithms and applications 
 *    to medical image processing", Olivier Cuisenaire's Ph.D. Thesis, October 
 *    1999, Université catholique de Louvain, Belgium.
 * 
 *  
 */
int *edt_saito(unsigned char *binary, int width, int height)
{
   int i,j,l,a,b,m, buffer, nsqr, diag1,
            *ptstart, *ptend, *sq, *df2, *buff, *pt, *npt;

   int *map = 0;
   sq = 0;
   buff = 0;

   
   map = malloc(width * height * sizeof(int));
   if(!map)
	   goto error_exit;
   for(i=0;i<width*height;i++)
     map[i] = binary[i];


   /* this is >= the diagonal minus 1 */
   diag1 = (int) ceil(sqrt((width-1)*(width-1)+(height-1)*(height-1))) - 1;

   /* Cuisenaire's idea: a LUT with precomputed i*i */
   nsqr = 2*(diag1 + 1);   /* was: 2*r + 2 in Cuisenaire's code */
   sq = malloc(nsqr * sizeof(int));
   if(!sq)
     goto error_exit;
   for(i=0; i<nsqr; i++)
      sq[i] = i*i;

   /* buff stores the current column in step 2 */
   buff = malloc(height * sizeof(int));
   if(!buff)
	   goto error_exit;

   /*-- Step 1 --*/
   for (j=0; j<height; j++) 
   {
      ptstart = map + j*width;
      ptend = ptstart + width;

      /* forward scan */
      df2 = sq + diag1;  /* the paper: df2 = sq + r, not large enough */
      for (pt=ptstart; pt < ptend; pt++)
         if (*pt)
            *pt = *(++df2);
         else
            df2 = sq;

      /* backward scan */
      df2 = sq + diag1;  /* the paper: df2 = sq + r, not large enough */
      for (--pt; pt != ptstart-1; --pt)
         if (*pt) 
		 {
            if (*pt > *(++df2))
               *pt = *df2;
         } 
		 else
           df2 = sq;
   }


   /*-- Step 2 --*/

   for (i=0; i<width; i++) 
   {
      pt = map + i;

      for (j=0; j<height; j++, pt+=width)
         buff[j]=*pt;

      pt = map + i + width;
      a  = 0;  
      buffer = buff[0];
      for (j=1;  j < height;  j++, pt+=width) 
	  {
         if (a != 0)
            --a;
         if (buff[j] > buffer+1) 
		 {
            b = (buff[j] - buffer-1) / 2;
            if (j+b+1 > height)
               b = height-1 -j;

            npt = pt + a*width;
            for (l=a; l<=b; l++) 
			{
               m = buffer + sq[l+1];
               if (buff[j+l] <= m)
                  break;   /* go to next column j */
               if (m < *npt)
                  *npt = m;
               npt += width;
            }
            a = b;
         } else
            a = 0;
         buffer = buff[j];
      }


      a = 0;
      pt -= 2*width;
      buffer = buff[height-1];

      for (j=height-2;  j != -1;  j--, pt-=width) 
	  {
         if (a != 0)
            --a;
         if (buff[j] > buffer+1) 
		 {
            b = (buff[j] - buffer-1) / 2;
            if (j < b)
               b = j;

            npt = pt - a*width;
            for (l=a; l<=b; ++l) 
			{
               m = buffer + sq[l+1];
               if (buff[j-l] <= m)
                  break;   /* go to next column j */
               if (m < *npt)
                  *npt = m;
               npt -= width;
            }
            a = b;
         } 
		 else
            a = 0;
         buffer = buff[j];
      }

   }

   free(sq);
   free(buff);
   
   return map;

error_exit:
   free(buff);
   free(sq);
   free(map);
   return 0;
}

