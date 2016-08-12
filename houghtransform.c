#include <math.h>

#define DEG2RAD 3.14/180.0

#define PI 3.1415926535897932384626433832795
#define degtorad(degrees) ( (degrees)/180.0 *PI)

int *houghtransform(unsigned char *binary, int width, int height, int *anglesout, int *hout)
{
	int x, y;
	int t;
	double hough_h = ((sqrt(2.0) * (height > width ? height : width)) / 2.0);
	int accu_h = (int) (hough_h * 2.0 + 0.5); // -r -> +r  
	int accu_w = 180;
	int *accumulator = 0;
	double centre_x, centre_y;
	
	accumulator = malloc(accu_h * accu_w, sizeof(int));
	if (!accumulator)
		goto error_exit;

	for (y = 0; y < accu_h; y++)
		for (x = 0; x < accu_w; x++ )
			accumulator[y*accu_w + x] = 0;

	centre_x = width / 2.0;
	centre_y = height / 2.0;


	for (y = 0; y < height; y++)
	{
		for (x = 0; x < width; x++)
		{
			if (binary[y*width + x] > 0)
			{
				for (t = 0; t < 180; t++)
				{
					double r = ((x - centre_x) * cos(degtorad(t))) + ((y - centre_y) * sin(degtorad(t)));
					accumulator[(int)((floor(r + hough_h+0.5) * 180.0)) + t]++;
				}
			}
		}
	}

	if (anglesout)
		*anglesout = accu_w;
	if (hout)
		*hout = accu_h;
    
	return accumulator;

error_exit:
	free(accumulator);
	return 0;
}

/*
   return line candidates as {x1, y1, x2, y2} * Nlines 
*/
int *hough_getlines(unsigned char *binary, int width, int height, int threshold, int *Nlines)
{
	int *accu;
	int _accu_h, _accu_w;
	int *answer = 0;
	int Nfound = 0;
	int r, t;
	int lx, ly;

	accu = houghtransform(binary, width, height, &_accu_w, &_accu_h);
	
	if (accu == 0)
		goto error_exit;
	
	for (r = 0; r<_accu_h; r++)
    {
		for (t = 0; t<_accu_w; t++)
	    {
		    if (accu[(r*_accu_w) + t] >= threshold)
		    {
			    /* Is this point a local maxima (9x9) */  
				int max = accu[(r*_accu_w) + t];
				for (ly = -4; ly <= 4; ly++)
			    {
					for (lx = -4; lx <= 4; lx++)
				    {
					    if ((ly + r >= 0 && ly + r<_accu_h) && (lx + t >= 0 && lx + t<_accu_w))
					    {
							if (accu[((r + ly)*_accu_w) + (t + lx)] > max)
						    {
								max = accu[((r + ly)*_accu_w) + (t + lx)];
								ly = lx = 5;
							}
					    }
					}
				}
				if (max > accu[(r*_accu_w) + t])
					continue;
				
				
				int x1, y1, x2, y2;
				x1 = y1 = x2 = y2 = 0;
				
				if (t >= 45 && t <= 135)
			    {
				                                //y = (r - x cos(t)) / sin(t)  
					x1 = 0;
					y1 = ((double)(r - (_accu_h / 2)) - ((x1 - (width / 2)) * cos(t * DEG2RAD))) / sin(t * DEG2RAD) + (height / 2);
					x2 = width - 1;
					y2 = ((double)(r - (_accu_h / 2)) - ((x2 - (width / 2)) * cos(t * DEG2RAD))) / sin(t * DEG2RAD) + (height / 2);
				}
				else
			    {
					                                //x = (r - y sin(t)) / cos(t);  
					y1 = 0;
				    x1 = ((double)(r - (_accu_h / 2)) - ((y1 - (height / 2)) * sin(t * DEG2RAD))) / cos(t * DEG2RAD) + (width / 2);
					y2 = height - 1;
				    x2 = ((double)(r - (_accu_h / 2)) - ((y2 - (height / 2)) * sin(t * DEG2RAD))) / cos(t * DEG2RAD) + (width / 2);
		        }
				
				answer = realloc(answer, (Nfound + 1) * 4 * sizeof(int));
				answer[Nfound * 4] = x1;
				answer[Nfound * 4 + 1] = y1;
				answer[Nfound * 4 + 2] = x2;
				answer[Nfound + 4 * 3] = y2;
			}
		 }
     }
	
	if (Nlines)
		*Nlines = Nfound;
	free(accu);
	return answer;      
error_exit:
	free(accu);
	free(answer);
	if (Nlines)
		*Nlines = -1;

	return 0;
}