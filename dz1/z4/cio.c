#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define MAXLINE 128
#define PIXPERLINE 16

char c[MAXLINE];

void pgmsize(char *filename, int *nx, int *ny)
{ 
  FILE *fp;

  if (NULL == (fp = fopen(filename,"r")))
  {
    fprintf(stderr, "pgmsize: cannot open <%s>\n", filename);
    exit(-1);
  }

  fgets(c, MAXLINE, fp);
  fgets(c, MAXLINE, fp);

  fscanf(fp,"%d %d",nx,ny);

  fclose(fp);
}


void pgmread(char *filename, void *vp, int nxmax, int nymax, int *nx, int *ny)
{ 
  FILE *fp;

  int nxt, nyt, i, j, t;

  int *pixmap = (int *) vp;

  if (NULL == (fp = fopen(filename,"r")))
  {
    fprintf(stderr, "pgmread: cannot open <%s>\n", filename);
    exit(-1);
  }

  fgets(c, MAXLINE, fp);
  fgets(c, MAXLINE, fp);

  fscanf(fp,"%d %d",nx,ny);

  nxt = *nx;
  nyt = *ny;

  if (nxt > nxmax || nyt > nymax)
  {
    fprintf(stderr, "pgmread: image larger than array\n");
    fprintf(stderr, "nxmax, nymax, nxt, nyt = %d, %d, %d, %d\n",
	    nxmax, nymax, nxt, nyt);
    exit(-1);
  }

  fscanf(fp,"%d", &t);

  for (j=0; j<nyt; j++)
  {
    for (i=0; i<nxt; i++)
    {
      fscanf(fp,"%d", &t);
      pixmap[(nyt-j-1)+nyt*i] = t;
    }
  }

  fclose(fp);
}

void pgmwrite(char *filename, void *vx, int nx, int ny)
{
  FILE *fp;

  int i, j, k, grey;

  double xmin, xmax, tmp;
  double thresh = 255.0;

  double *x = (double *) vx;

  if (NULL == (fp = fopen(filename,"w")))
  {
    fprintf(stderr, "pgmwrite: cannot create <%s>\n", filename);
    exit(-1);
  }

  xmin = fabs(x[0]);
  xmax = fabs(x[0]);

  for (i=0; i < nx*ny; i++)
  {
    if (fabs(x[i]) < xmin) xmin = fabs(x[i]);
    if (fabs(x[i]) > xmax) xmax = fabs(x[i]);
  }

  fprintf(fp, "P2\n");
  fprintf(fp, "# Written by pgmwrite\n");
  fprintf(fp, "%d %d\n", nx, ny);
  fprintf(fp, "%d\n", (int) thresh);

  k = 0;

  for (j=ny-1; j >=0 ; j--)
  {
    for (i=0; i < nx; i++)
    {
 
      tmp = x[j+ny*i];

 
      if (xmin < 0 || xmax > thresh)
      {
        tmp = (int) ((thresh*((fabs(tmp-xmin))/(xmax-xmin))) + 0.5);
      }
      else
      {
        tmp = (int) (fabs(tmp) + 0.5);
      }

      grey = tmp;
 
      fprintf(fp, "%3d ", grey);

      if (0 == (k+1)%PIXPERLINE) fprintf(fp, "\n");

      k++;
    }
  }

  if (0 != k%PIXPERLINE) fprintf(fp, "\n");
  fclose(fp);
}
