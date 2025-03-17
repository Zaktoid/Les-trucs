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
float CountN(int k,int l,int* tab,int width)
{
    float s=0;
    for(int i=-1;i<=1;i++)
        for(int j=-1;j<=1;j++)
        {
         s+=tab[(k+i)*width+l+j]*(i!=0||j!=0);   
        }
    return s;
}


int main(int argc , char **argv)
{
	srand(time(NULL));
	int hauteur=800;
    int Largeur=800;
    int pause=0;
    int t=0;
	SDL_Window *window;
	SDL_Renderer *rendu;
	if(SDL_Init(SDL_INIT_VIDEO))
		SDL_Exitwitherror("failed init");
	window= SDL_CreateWindow("Nom fenêtre",SDL_WINDOWPOS_CENTERED,0,Largeur,hauteur,0);			
	if(!window)
		SDL_Exitwitherror("window creation failed");
	rendu = SDL_CreateRenderer(window,-1,SDL_RENDERER_SOFTWARE);
	if(!rendu)
		SDL_Exitwitherror("renderer failed");
	SDL_bool Launched= SDL_TRUE;
	SDL_SetRenderDrawColor(rendu,180,50,70,255);
    int* Cells=malloc(sizeof(int)*Largeur*hauteur);
    int* NCells=malloc(sizeof(int)*Largeur*hauteur);
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
                case SDLK_p:
                    pause=1-pause;
				default:
					break;
				}
                break;
            case SDL_MOUSEBUTTONDOWN:
                t=1;
                break;
            case SDL_MOUSEMOTION:
                if(t)
                {   
                    int x=event.button.x;
                    int y=event.button.y;
                    NCells[x*hauteur+y]=1;
                }
                break;
            case SDL_MOUSEBUTTONUP:
                t=0;
                break;
			default:
				break;
		}}
        if(!pause)
        {
            for(int i=1;i<(Largeur-1);i++)
            {
                for(int j=1;j<hauteur-1;j++)
                {
                    float u=CountN(i,j,Cells,hauteur);
                    if(Cells[i*hauteur+j])
                    {    
                        if(u>=2 && u<=10) NCells[i*hauteur+j]+=4;
                        else NCells[i*hauteur+j]-=5;
                    }
                    else
                        if(u>=10) NCells[i*hauteur+j]+=5;
                }
            }
        }
            for(int i=1;i<(Largeur-1);i++)
            {
                for(int j=1;j<hauteur-1;j++)
                {
                    float s=NCells[i*hauteur+j];
                    SDL_SetRenderDrawColor(rendu,105*s,40*s,120*s,255);
                    SDL_RenderDrawPoint(rendu,i,j);
                    Cells[i*hauteur+j]=NCells[i*hauteur+j];
                }
            }
        
		SDL_RenderPresent(rendu);



}
/*************************************/

	SDL_DestroyRenderer(rendu);
	SDL_DestroyWindow(window);
	SDL_Quit();
	return EXIT_SUCCESS;

}