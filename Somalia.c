#include <stdlib.h>
#include <stdio.h>
#include <SDL2/SDL.h>
#include <math.h>
#include <time.h>
#include "Mathutils.h"

float Bisext(float(*f)(float),float d_min,float d_max,float toler)
{
	float a=d_min;
	float b=d_max;
	float guess=(a-b)/2;
	while (f(guess)>toler)
	{
		if(f(guess)>0)
			b=guess;
		else
			a=guess;
		guess=(a-b)/2;
					
	}
	return guess;
	
}

int main(int argc , char **argv)
{



}