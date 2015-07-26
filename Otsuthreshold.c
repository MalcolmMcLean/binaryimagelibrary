/*
  get the Otusu threshold for image segmentation
  Params: gray - the grayscale image
          width - image width
          height - uimage height
  Returns: threshold at which to split pixels into foreground and
           background.
 */
int getOtsuthreshold(unsigned char *grey, int width, int height)
{
  int hist[256] = {0};
  int wB = 0;
  int wF;
  float mB, mF;
  float sum = 0;
  float sumB = 0;
  float varBetween;
  float varMax = 0.0f;
  int answer = 0;
  int i;
  int k;
  
  for(i=0;i<width*height;i++)
    hist[grey[i]]++;
 
  /* sum of all (for means) */
  for (k=0 ; k<256 ; k++) 
       sum += k * hist[k];

  for(k=0;k<256;k++)
  {
     wB += hist[k];               
     if (wB == 0) 
         continue;

     wF = width*height - wB;            
     if (wF == 0) 
       break;

     sumB += (float) (k * hist[k]);

     mB = sumB / wB;            /* Mean Background */
     mF = (sum - sumB) / wF;    /* Mean Foreground */

     /* Calculate Between Class Variance */
     varBetween = (float)wB * (float)wF * (mB - mF) * (mB - mF);

     /* Check if new maximum found */
     if (varBetween > varMax) 
     {
       varMax = varBetween;
       answer = k;
     }

  }
  return answer;
}
