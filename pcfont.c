/**@file

  Code for a PC-like font.

  Use to quickly build up raster displays.

*/

#include "pcfont8x19.h"
#include "pcfont8x8.h"

/* http://dwarffortresswiki.org/index.php/Tileset_repository */

#define clamp(x,low,high) ( (x) < (low) ? (low) : (x) > (high) ? (high) : (x) ) 

/**
  Write a character to the binary buffer using 8x8 font

   @param[in,out] binary - the buffer
   @param width - bufffer width
   @param height - buffer height
   @param x - pixel x position (left)
   @param y - pixel y position (top)
   @param ch - character to write
   @returns 0 on success, -1 on fail
*/
int cgafont_paste(unsigned char *binary, int width, int height, int x, int y, int ch)
{
  int i, ii;
  int lowx, highx, lowy, highy;
  char *glyph;

  if(ch < 0 || ch >= 256)
    return -1;

  glyph = pcfont8x8[ch];

  lowx = clamp(x, 0, width-1);
  highx = clamp(x+8, 0, width-1);
  lowy = clamp(y, 0, height-1);
  highy = clamp(y+8, 0, height-1);


  for(i=lowy;i<highy;i++)
    for(ii=lowx;ii<highx;ii++)
	{
		binary[i*width+ii] = glyph[(i-y)*8+ii-x] == '#' ? 1 : 0; 
	}

  return 0;
}

/**
Write a character to the binary buffer, using 8x19 font

@param[in,out] binary - the buffer
@param width - bufffer width
@param height - buffer height
@param x - pixel x position (left)
@param y - pixel y position (top)
@param ch - character to write
@returns 0 on success, -1 on fail
*/
int pcfont_paste(unsigned char *binary, int width, int height, int x, int y, int ch)
{
  int i, ii;
  int lowx, highx, lowy, highy;
  char *glyph;

  if(ch < 0 || ch >= 256)
    return -1;

  glyph = pcfont8x19[ch];

  lowx = clamp(x, 0, width-1);
  highx = clamp(x+8, 0, width-1);
  lowy = clamp(y, 0, height-1);
  highy = clamp(y+19, 0, height-1);


  for(i=lowy;i<highy;i++)
    for(ii=lowx;ii<highx;ii++)
	{
		binary[i*width+ii] = glyph[(i-y)*8+ii-x] == '#' ? 1 : 0; 
	}

  return 0;
}

/**
Write a character to the binary buffer using 8x8 font

@param[in,out] binary - the buffer
@param width - bufffer width
@param height - buffer height
@param x - character x position (left)
@param y - character y position (top)
@param ch - character to write
@returns 0 on success, -1 on fail
*/
int cgafont_putch(unsigned char *binary, int chwidth, int chheight, int cx, int cy, int ch)
{
	return cgafont_paste(binary, chwidth * 8, chheight * 8, cx * 8, cy * 8, ch);
}

/**
Write a character to the binary buffer using 8x8 font

@param[in,out] binary - the buffer
@param width - bufffer width
@param height - buffer height
@param x - character x position (left)
@param y - character y position (top)
@param ch - character to write
@returns 0 on success, -1 on fail
*/
int pcfont_putch(unsigned char *binary, int chwidth, int chheight, int cx, int cy, int ch)
{
	return pcfont_paste(binary, chwidth * 8, chheight * 19, cx * 8, cy * 19, ch);
}


