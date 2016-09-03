#include <stdlib.h>
#include <string.h>
/**@file

  Basic drawing routines for binary images.

  Unlikely to want to draw many binary images for human viewing,
  but the roi=utines are useful for itnermediates steps. They
  are also good simple reference implementations of algorithms.
*/

/**
  Draw a line using Brasenham's algorithm.

   @param[in,out] binary - the binary image
   @param width - image width
   @param height - image height
   @param x0 - start x co-ordinate
   @param y0 - start y co-ordiante
   @param x1 - end x co-ordinate
   @param y1 - end y co-ordinate
*/
void binaryline(unsigned char *binary, int width, int height, int x0, int y0, int x1, int y1)
{
   int dx, dy;
   int sx, sy;
   int e2, err;

   dx = abs(x1-x0);
   dy = abs(y1-y0);
   if(x0 < x1)
	   sx = 1;
   else sx = -1;
   if(y0 < y1)
	  sy = 1;
   else sy = -1;
   err = dx-dy;
 
   while(1)
   {
	 if(x0 >= 0 && x0 < width && y0 >= 0 && y0 < height)
	    binary[y0*width+x0] = 1;
     if(x0 == x1 && y0 == y1)
		 break;
     e2 = 2*err;
     if(e2 > -dy)
	 {
       err = err - dy;
       x0 = x0 + sx;
	 }
     if(e2 <  dx)
	 {
       err = err + dx;
       y0 = y0 + sy; 
	 }
   }
 }

/**
  Draw an open circle using quadrant mirroring

  @param[in,out] binary - the binary image
  @param width - image width
  @param height - image height
  @param xm - origin x
  @param ym - origin y
  @param r - radius.

  Notes: interger only algorithm
*/
void binarycircle(unsigned char *binary, int width, int height, int xm, int ym, int r)
{
   int x = -r, y = 0, err = 2-2*r; /* II. Quadrant */ 
   do {
      if(xm-x >= 0 && xm-x < width && ym+y >= 0 && ym+y < height)
        binary[xm-x + (ym+y)*width] = 1; /*   I. Quadrant */
	  if(xm-y >= 0 && xm-y < width && ym-x >= 0 && ym-x < height)
        binary[xm-y + (ym-x)*width] = 1; /*  II. Quadrant */
	  if(xm+x >= 0 && xm+x < width && ym-y >= 0 && ym-y < height)
        binary[xm+x + (ym-y) *width] = 1; /* III. Quadrant */
	  if(xm+y >= 0 && xm+y < width && ym+x >= 0 && ym+x < height)
        binary[xm+y +  (ym+x)*width] = 1; /*  IV. Quadrant */
      r = err;
      if (r >  x) 
		  err += ++x*2+1; /* e_xy+e_x > 0 */
      if (r <= y) 
		  err += ++y*2+1; /* e_xy+e_y < 0 */
   } while (x < 0);
}

/**
  Draw an ellipse.

  @param[in.out] binary - the binary image
  @param width - image width
  @param height - image height
  @param x0 - first focus x-co-ordinate
  @param y0 - first focus y co-ordiante
  @param x1 - second focus x co-ordinate
  @param y1 - second focus y co-ordiante

  Integer only algorithm.
*/
void binaryellipse(unsigned char *binary, int width, int height, int x0, int y0, int x1, int y1)
{
   int a = abs(x1-x0), b = abs(y1-y0), b1 = b&1; /* values of diameter */
   long dx = 4*(1-a)*b*b, dy = 4*(b1+1)*a*a; /* error increment */
   long err = dx+dy+b1*a*a, e2; /* error of 1.step */

   if (x0 > x1) { x0 = x1; x1 += a; } /* if called with swapped points */
   if (y0 > y1) y0 = y1; /* .. exchange them */
   y0 += (b+1)/2; y1 = y0-b1;   /* starting pixel */
   a *= 8*a; b1 = 8*b*b;

   do {
	   if(x1 >= 0 && x1 < width && y0 >= 0 && y0 < height)
         binary[y0*width+x1] = 1; /*   I. Quadrant */
	   if(x0 >= 0 && x0 < width && y0 >= 0 && y0 < height)
         binary[y0*width+x0] = 1;/*  II. Quadrant */
	   if(x0 >= 0 && x0 < width && y1 >= 0 && y1 < height)
         binary[y1*width+x0] = 1;; /* III. Quadrant */
	   if(x1 >= 0 && x1 < width && y1 >= 0 && y1 < height)
         binary[y1*width+x1] = 1; /*  IV. Quadrant */
       e2 = 2*err;
       if (e2 >= dx) { x0++; x1--; err += dx += b1; } /* x step */
       if (e2 <= dy) { y0++; y1--; err += dy += a; }  /* y step */ 
   } while (x0 <= x1);
   
   while (y0-y1 < b) {  /* too early stop of flat ellipses a=1 */
	   if(x0-1 >= 0 && x0-1 < width && y0 >= 0 && y0 < height) 
         binary[y0*width+x0-1] = 1; /* -> finish tip of ellipse */
	   if(x1+1 >= 0 && x1+1 < width && y0 >= 0 && y0 < height)
         binary[y0*width+x1+1] =1;
	    y0++;
	   if(x0-1 >= 0 && x0-1 < width && y1 >= 0 && y1 < height)
         binary[y1 * width + x0-1] = 1;
       if(x1+1 >= 0 && x1+1 < width && y1 >= 0 && y1 < height) 
	     binary[y1*width+x1+1] = 1;
	   y1--;
   }
}

/**
   Draw a cubic Bezier curve

   @param[in,out] biary - the bianry image
   @param width - image width
   @param height - image height
   @param[in] x - 4 Bezier control points x
   @param[in] y - 4 Bezier control points y

*/
void binarybezier(unsigned char *binary, int width, int height, float *x, float *y)
{
  float t = 0;
  float lastt;
  float dt;
  float tx, ty;
  int px, py;
  int lastx, lasty;

  dt = 0.05f;

  lastx = (int) (x[0] + 0.5f);
  lasty = (int) (y[0] + 0.5f);
  if(lastx >= 0 && lastx < width && lasty >= 0 && lasty < height)
    binary[lasty*width+lastx] = 1;

  lastt = 0;
  while(t < 1.0)
  {
    do
    {
	  t = lastt + dt;
	  tx = x[0]*(1-t)*(1-t)*(1-t) + x[1]*(1-t)*(1-t)*t + x[2]*(1-t)*t*t + x[3]*t*t*t;
	  ty = y[0]*(1-t)*(1-t)*(1-t) + y[1]*(1-t)*(1-t)*t + y[2]*(1-t)*t*t + y[3]*t*t*t;
	  px = (int) (tx + 0.5);
	  py = (int) (ty + 0.5);
	  if(t > 1 && px == lastx && py == lasty)
		  break;
	  if(abs(px-lastx) > 1 || abs(py-lasty) > 1)
	    dt /= 2; 
	  else if(px == lastx && py == lasty)
	    dt *= 1.5;
	  else
	  {
	    if(px >= 0 && px < width && py >= 0 && py < height)
			binary[py*width+px] = 1;
		lastx = px;
		lasty = py;
		break;
	  }
	  if(dt < 0.000001f)
      {
	    dt = 0.01f;
		t = t + dt;
		break;
	  }
	} while(1);
	lastt = t;
  }
}

/**
  Draw a Catmull-Rom curve

  @param[in,ut] binary - the binary image
  @param width - image width
  @param height - image height
  @param[in] x - Catmull-Rom x control points (at least 4)
  @param[in] y - Catmull-Rom y control oints (at least 4)

*/
 void binarycatmullrom(unsigned char *binary, int width, int height, float *x, float *y, int N)
 {
    float t, t2, t3;
	float lastt;
    int i;
    int px, py;
	int lastx, lasty;
	float dt;
	float tx, ty;

    for(i=0;i<N-3;i++)
	{
	  dt = 0.05f;

      lastx = (int) (x[1] + 0.5f);
      lasty = (int) (y[1] + 0.5f);
      if(lastx >= 0 && lastx < width && lasty >= 0 && lasty < height)
        binary[lasty*width+lastx] = 1;

      lastt = 0;
	  t = 0;
	  while(t < 1.0)
	  {
       do
	   {
	     t = lastt + dt;
	     t2 = t * t;
	     t3 = t * t * t;
         tx = 0.5f * ( ( 2.0f * x[1] ) +
           ( -x[0] + x[2] ) * t +
           ( 2.0f * x[0] - 5.0f * x[1] + 4 * x[2] - x[3] ) * t2 +
           ( -x[0] + 3.0f * x[1] - 3.0f * x[2] + x[3] ) * t3 );
         ty = 0.5f * ( ( 2.0f * y[1] ) +
           ( -y[0] + y[2] ) * t +
           ( 2.0f * y[0] - 5.0f * y[1] + 4 * y[2] - y[3] ) * t2 +
           ( -y[0] + 3.0f * y[1] - 3.0f * y[2] + y[3] ) * t3 );
	     px = (int) (tx + 0.5);
	     py = (int) (ty + 0.5);
	     if(t > 1 && px == lastx && py == lasty)
		   break;
	     if(abs(px-lastx) > 1 || abs(py-lasty) > 1)
	       dt /= 2;
	     else if(px == lastx && py == lasty)
	       dt *= 1.5;
	     else
	     {
	       if(px >= 0 && px < width && py >= 0 && py < height)
			 binary[py*width+px] = 1;
		   lastx = px;
		   lasty = py;
		   break;
	     }
		 if(dt < 0.000001f)
		 {
		   dt = 0.01f;
		   t = t + dt;
		   break;
		 }
	   }
	   while(1);
	   lastt = t;
	  }
	  x++;
	  y++;
   }
 } 

 /*
 Algorithm:
Flood-fill (node, target-color, replacement-color):
 1. Set Q to the empty queue.
 2. If the color of node is not equal to target-color, return.
 3. Add node to Q.
 4. For each element n of Q:
 5.     If the color of n is equal to target-color:
 6.         Set w and e equal to n.
 7.         Move w to the west until the color of the node to the west of w no longer matches target-color.
 8.         Move e to the east until the color of the node to the east of e no longer matches target-color.
 9.         Set the color of nodes between w and e to replacement-color.
10.         For each node n between w and e:
11.             If the color of the node to the north of n is target-color, add that node to Q.
12.             If the color of the node to the south of n is target-color, add that node to Q.
13. Continue looping until Q is exhausted.
14. Return.
*/

/**
  Floodfill4 - floodfill, 4 connectivity.

  @param[in,out] grey - the image (formally it's greyscale but it could be binary or indexed)
  @param width - image width
  @param height - image height
  @param x - seed point x
  @param y - seed point y
  @param target - the colour to flood
  @param dest - the colur to replace it by.
  @returns Number of pixels flooded.
*/
int floodfill4(unsigned char *grey, int width, int height, int x, int y, unsigned char target, unsigned char dest)
{
  int *qx = 0;
  int *qy = 0;
  int qN = 0;
  int qpos = 0;
  int qcapacity = 0;
  int wx, wy;
  int ex, ey;
  int tx, ty;
  int ix;
  int *temp;
  int answer = 0;

  if(grey[y * width + x] != target)
    return 0;
  qx = malloc(width * sizeof(int));
  qy = malloc(width * sizeof(int));
  if(qx == 0 || qy == 0)
    goto error_exit;
  qcapacity = width;
  qx[qpos] = x;
  qy[qpos] = y;
  qN = 1; 

  while(qN != 0)
  {
    tx = qx[qpos];
    ty = qy[qpos];
    qpos++;
    qN--;
   
    if(qpos == 256)
    {
      memmove(qx, qx + 256, qN*sizeof(int));
      memmove(qy, qy + 256, qN*sizeof(int));
      qpos = 0;
    }
    if(grey[ty*width+tx] != target)
      continue;
    wx = tx;
    wy = ty;
    while(wx >= 0 && grey[wy*width+wx] == target)
      wx--;
    wx++;
    ex = tx;
    ey = ty;
    while(ex < width && grey[ey*width+ex] == target)
      ex++;
    ex--; 
     

    for(ix=wx;ix<=ex;ix++)
    {
      grey[ty*width+ix] = dest;
      answer++;
    }

    if(ty > 0)
      for(ix=wx;ix<=ex;ix++)
      {
	    if(grey[(ty-1)*width+ix] == target)
	    {
          if(qpos + qN == qcapacity)
	      {
            temp = realloc(qx, (qcapacity + width) * sizeof(int));
            if(temp == 0)
              goto error_exit;
            qx = temp;
            temp = realloc(qy, (qcapacity + width) * sizeof(int));
            if(temp == 0)
              goto error_exit;
            qy = temp;
            qcapacity += width;
	      }
          qx[qpos+qN] = ix;
          qy[qpos+qN] = ty-1;
          qN++;
	    }
      }
    if(ty < height -1)
      for(ix=wx;ix<=ex;ix++)
      {
        if(grey[(ty+1)*width+ix] == target)
	    {
          if(qpos + qN == qcapacity)
	      {
            temp = realloc(qx, (qcapacity + width) * sizeof(int));
            if(temp == 0)
              goto error_exit;
            qx = temp;
            temp = realloc(qy, (qcapacity + width) * sizeof(int));
            if(temp == 0)
              goto error_exit;
            qy = temp;
            qcapacity += width;
	      }
          qx[qpos+qN] = ix;
          qy[qpos+qN] = ty+1;
          qN++; 
	    }
      } 
  }

  free(qx);
  free(qy);

  return answer;
 error_exit:
  free(qx);
  free(qy);
  return -1;
}

/*
  Floodfill, 8 connectivity.

  @param[in,out] grey - the image (formally it's greyscale but it could be binary or indexed)
  @param width - image width
  @param height - image height
  @param x - seed point x
  @param y - seed point y
  @param target - the colour to flood
  @param dest - the colur to replace it by.
  @returns Number of pixels flooded.

*/
int floodfill8(unsigned char *grey, int width, int height, int x, int y, unsigned char target, unsigned char dest)
{
  int *qx = 0;
  int *qy = 0;
  int qN = 0;
  int qpos = 0;
  int qcapacity = 0;
  int wx, wy;
  int ex, ey;
  int tx, ty;
  int ix;
  int *temp;
  int answer = 0;

  if(grey[y * width + x] != target)
    return 0;
  qx = malloc(width * sizeof(int));
  qy = malloc(width * sizeof(int));
  if(qx == 0 || qy == 0)
    goto error_exit;
  qcapacity = width;
  qx[qpos] = x;
  qy[qpos] = y;
  qN = 1; 

  while(qN != 0)
  {
    tx = qx[qpos];
    ty = qy[qpos];
    qpos++;
    qN--;
   
    if(qpos == 256)
    {
      memmove(qx, qx + 256, qN*sizeof(int));
      memmove(qy, qy + 256, qN*sizeof(int));
      qpos = 0;
    }
    if(grey[ty*width+tx] != target)
      continue;
    wx = tx;
    wy = ty;
    while(wx >= 0 && grey[wy*width+wx] == target)
      wx--;
    wx++;
    ex = tx;
    ey = ty;
    while(ex < width && grey[ey*width+ex] == target)
      ex++;
    ex--; 
     

    for(ix=wx;ix<=ex;ix++)
    {
      grey[ty*width+ix] = dest;
      answer++;
    }

    if(ty > 0)
      for(ix=wx-1;ix<=ex+1;ix++)
      {
        if(ix < 0 || ix >= width)
	  continue;
	if(grey[(ty-1)*width+ix] == target)
	{
          if(qpos + qN == qcapacity)
	  {
            temp = realloc(qx, (qcapacity + width) * sizeof(int));
            if(temp == 0)
              goto error_exit;
            qx = temp;
            temp = realloc(qy, (qcapacity + width) * sizeof(int));
            if(temp == 0)
              goto error_exit;
            qy = temp;
            qcapacity += width;
	  }
          qx[qpos+qN] = ix;
          qy[qpos+qN] = ty-1;
          qN++;
         
	}
      }
    if(ty < height -1)
      for(ix=wx-1;ix<=ex+1;ix++)
      {
        if(ix < 0 || ix >= width)
	  continue;

        if(grey[(ty+1)*width+ix] == target)
	{
          if(qpos + qN == qcapacity)
	  {
            temp = realloc(qx, (qcapacity + width) * sizeof(int));
            if(temp == 0)
              goto error_exit;
            qx = temp;
            temp = realloc(qy, (qcapacity + width) * sizeof(int));
            if(temp == 0)
              goto error_exit;
            qy = temp;
            qcapacity += width;
	  }
          qx[qpos+qN] = ix;
          qy[qpos+qN] = ty+1;
          qN++;
        
	}
      } 
  }

  free(qx);
  free(qy);

  return answer;
 error_exit:
  free(qx);
  free(qy);
  return -1;
}
