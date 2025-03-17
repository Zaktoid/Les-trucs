#include <stdlib.h>
#include <stdio.h>
#include <SDL2/SDL.h>
#include <math.h>
#include <time.h>
#include "Mathutils.h"



double Itertive(double (*f)(double),double a, double b,double prec)
{

    if(f(a)*f(b)>=0)
        {
            printf("Invalid input");
            return 0;
        }
    double c=(a+b)/2.0;
    while (fabs(f(c))>=prec)
    {
        if(f(a)*f(c)<0)
            {b=c;a=a;c=(a+b)/2.0;}
        if(f(a)*f(c)>0)
            {a=c;b=b;c=(a+b)/2.0;}
        printf("%f \n",c);
    }
    return c;
    


}

double g(double a)
{
    return sin(a)-cos(a);
}

int main(int argc , char **argv)
{

double a=Itertive(g,0,1,0.000000000001);
printf("%.15lf \n",a);

}