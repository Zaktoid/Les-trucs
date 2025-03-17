#include <stdlib.h>
#include <stdio.h>
#include <SDL2/SDL.h>
#include <math.h>
#include <time.h>
#include "Mathutils.h"



void SDL_Exitwitherror(const char *message)
{
	SDL_Log("Erreur:%s >%s \n",message,SDL_GetError());
	exit(EXIT_FAILURE);
}
float Image(float x,float y)
{
    if(0.25<=x && x<=0.5 && 0.25<=y && y<=0.5 )
        return 1;
    return 0;
}
float G(float x,float y,int j,int k)
    {
        return cos(2*3.14*(j*x+k*y))*Image(x,y);
    }

float H(float x,float y,int j,int k)
    {
        return sin(2*3.14*(j*x+k*y))*Image(x,y);
    }

float FourierRect(int a, int b,float x,float y)
{
    float out=
    return out;
}









int main(int argc , char **argv)
{
    
	srand(time(NULL));
	int hauteur=300;
	SDL_Window *window;
	SDL_Renderer *rendu;
	if(SDL_Init(SDL_INIT_VIDEO))
		SDL_Exitwitherror("failed init");
	window= SDL_CreateWindow("Nom fenêtre",SDL_WINDOWPOS_CENTERED,0,hauteur,hauteur,0);			
	if(!window)
		SDL_Exitwitherror("window creation failed");
	rendu = SDL_CreateRenderer(window,-1,SDL_RENDERER_SOFTWARE);
	if(!rendu)
		SDL_Exitwitherror("renderer failed");
	SDL_bool Launched= SDL_TRUE;
	SDL_SetRenderDrawColor(rendu,255,0,0,255);
    float a=1;
    float b=1;
    float e=0.1;
    float f=0;
    int N=40;
    while (Launched)
	{
		SDL_Event event;
		while(SDL_PollEvent(&event))
		{
			switch (event.type)
			{
			case SDL_QUIT:
				Launched= SDL_FALSE;
				break;
			case SDL_KEYDOWN:
				switch (event.key.keysym.sym)
				{
                case SDLK_a:
                    a+=0.01;
                    break;
                case SDLK_z:
                    a-=0.01;
                    break;
                case SDLK_b:
                    b+=0.01;
                    break;
                case SDLK_n:
                    b-=0.01;
                    break;
                case SDLK_e:
                    e+=0.01;
                    break;
                case SDLK_f:
                    f+=0.01;
                    break;
                case SDLK_g:
                    f-=0.01;
                    break;
				default:
					break;
				}
			default:
				break;
		}}
	for(int i=0;i<hauteur/2;i++)
	{
        for(int j=0;j<hauteur/2;j++)
        {
            float x=2*(float)i/hauteur;
            float y=2*(float)j/hauteur;
            if(0.25<=x && x<=0.25)
            SDL_SetRenderDrawColor(rendu,s*255,s*255,s*255,255);
        }

	}
		SDL_RenderPresent(rendu);


}
/*************************************/

    //printf("%f,%f,%f,%f \n",a,b,e,f);
    printf("%f \n",Integ2D(H,30,1,1));
	SDL_DestroyRenderer(rendu);
	SDL_DestroyWindow(window);
	SDL_Quit();
	return EXIT_SUCCESS;

}