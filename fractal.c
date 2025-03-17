#include <stdido.h>
#include <stdlib.h>
#include <draw.h>


void DrawNgone(int xCentre,int yCentre,int rayon,int n,int r,int g , int b)
{
	double a=2*(M_PI)/n;
	for (int i=0;i<n;i++)
	{
		draw_line(xCentre+rayon*cos(a*i),yCentre+rayon*sin(a*i),xCentre+rayon*cos(a*(1+i)),yCentre+rayon*sin(a*(i+1)),1,r,g,b);
	}
}


int main(int arc, char **argv)
    {

	int r=10;
	draw_init("test.svg", 400, 400);
	for (int l=1;r*2*l<400;l++)
	{
			for (int k=1;k*2*2<400;k++)
		
		{
			DrawNgone(k*2*r,r*2*l,r,k+2,0,40*l,0);
			k++;
		}
	}
	draw_close();

    }