#ifndef binaryutils_h
#define binaryutils_h
int morphclose(unsigned char *binary, int width, int height, unsigned char *sel, int swidth, int sheight);
int morphopen(unsigned char *binary, int width, int height, unsigned char *sel, int swidth, int sheight);
int dilate(unsigned char *binary, int width, int height, unsigned char *sel, int swidth, int sheight);
int erode(unsigned char *binary, int width, int height, unsigned char *sel, int swidth, int sheight);
int *labelconnected(unsigned char *binary, int width, int height, int connex, int *Nout);
int eulernumber(unsigned char *binary, int width, int height);
int getbiggestobject(unsigned char *binary, int width, int height, int connex);
int branchpoints(unsigned char *binary, int width, int height, int **xout, int **yout);
int lineends(unsigned char *binary, int width, int height, int **xout, int **yout);
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

#endif
