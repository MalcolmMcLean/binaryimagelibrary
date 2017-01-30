//
//  featuretransform.c
//  
//
//  Created by Malcolm McLean on 16/01/2017.
//
//

#include "featuretransform.h"
#include <stdlib.h>
#include <string.h>


static int *processRows(unsigned char *binary, int width, int height);
static int *processColumns(int *ftRow, int width, int height);
static int separation(int *ftRow, int width, int height, int iRow, int uRow, int colR2 );
static int distance(int *ftRow, int width, int height, int xRow, int iRow, int colR2 );

/**
   feature transform - like distance transform except gets 
      the co-ordinates of the nearest background pixel
 
    Returns: feature transfrom [2][height][width] with rows
       in the first half of the buffer and columns in the second.
 
 */
int *featuretransform(unsigned char *binary, int width, int height)
{
    int *ftRow;
    int *ft;
    
    ftRow = processRows(binary, width, height);
    ft = processColumns(ftRow, width, height);
    
    return ft;
}



// Algorithm in two rounds; round one: along rows
static int *processRows(unsigned char *binary, int width, int height) {
    // feature transform array - row only
    int *ftRow = malloc(width * height * sizeof(int));
    
    // temporary column distance array
    int *rowDist = malloc(width * sizeof(int));
    memset(rowDist, 0, width * sizeof(int));
    
    for ( int row=0; row<height; row++ )
    {
        int startRow = row*width;
        // Count distance from right boundary
        if ( binary[startRow+width-1] ) {
            rowDist[width-1] = 0;
        } else {
            rowDist[width-1] = width+height/*inf*/;
        }
        for ( int col=width-2; col>=0; col-- ) {
            if ( binary[startRow+col] ) {
                rowDist[col] = 0;
            } else {
                rowDist[col] = 1 + rowDist[col+1];
            }
        }
        // column for first pixel is equal distance
        ftRow[startRow] = rowDist[0];
        // Update with distance from left boundary
        for ( int col=1; col<width; col++ ) {
            if ( col - ftRow[startRow+col-1] <= rowDist[col] ) {
                ftRow[startRow+col] = ftRow[startRow+col-1];
            } else {
                ftRow[startRow+col] = col + rowDist[col];
            }
        }
    }
    free(rowDist);
    return ftRow;
}


// Algorithm in two rounds; round two: along cols
static int *processColumns(int *ftRow, int width, int height)
{
    int *ft = malloc(2 * width * height * sizeof(int));
    int *seg = malloc( (height + 1) * sizeof(int)); // s
    int *val = malloc( (height + 1) * sizeof(int)); // t
    int idSeg; // q
    memset(seg, 0, (height +1) * sizeof(int));
    memset(val, 0, (height+1) * sizeof(int));
    for (int colR2=0; colR2 < width; colR2++ ) {
        idSeg = 0;
        seg[0] = 0;
        val[0] = 0;
        
        for ( int u=1; u < height; u++ ) {
            while ( idSeg >= 0 &&
                   distance(ftRow, width, height, val[idSeg],seg[idSeg], colR2) >
                   distance(ftRow, width, height, val[idSeg],u, colR2)) {
                --idSeg;
            }
            if ( idSeg < 0 ) {
                idSeg = 0;
                seg[idSeg] = u; // set current index to be first
            } else {
                int curVal = 1 + separation(ftRow, width, height, seg[idSeg],u, colR2);
                // Check if current segment becomes minimal inside image
                if ( curVal < height ) {
                    // Update for current segment
                    ++idSeg;
                    seg[idSeg] = u;
                    val[idSeg] = curVal;
                }
            }
        }
        for ( int u=height-1; u>=0; u-- ) {
            ft[0*width*height+u*width+colR2] =
            ftRow[seg[idSeg]*width+colR2];
            ft[1*width*height+u*width+colR2] = seg[idSeg];
            // Reached next segment?
            if ( u==val[idSeg] ) {
                --idSeg;
            }
        }
    }
    return ft;
}


/** Calculates row where iSeg(row) <= uSeg(row)
 Override function for different distance metric */
static int separation(int *ftRow, int width, int height, int iRow, int uRow, int colR2 )
{
    if ( uRow - iRow == 0 ) return 0;
    int iRowDist = colR2-ftRow[iRow*width+colR2];
    int uRowDist = colR2-ftRow[uRow*width+colR2];
    return ((uRow*uRow-iRow*iRow
             + uRowDist*uRowDist - iRowDist*iRowDist)/
            (2*(uRow-iRow)));
}


/** Distance from xRow to boundary via iRow
 Override function for different distance metric */
static int distance(int *ftRow, int width, int height, int xRow, int iRow, int colR2 ) {
    int rowDist = 0;

    rowDist = colR2 - ftRow[iRow*width+colR2];

    
    return (xRow-iRow)*(xRow-iRow)
    + rowDist*rowDist;
}

