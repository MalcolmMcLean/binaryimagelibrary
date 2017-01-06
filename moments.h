#ifndef moments_h
#define moments_h

typedef struct
{
  double mu_00;
  double mu_11;
  double mu_20;
  double mu_02;
  double mu_21;
  double mu_12;
  double mu_30;
  double mu_03;
  double cov[2][2];
  double eccentricity;
  double theta;
  double area;
  double xbar;
  double ybar;
} MOMENTS;

void binarymoments(unsigned char *binary, int width, int height, MOMENTS *ret);

#endif
