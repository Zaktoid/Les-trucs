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
int CountN(int *Cells,int x , int y,int taille)
{
	int s=0;
	for(int i=-1;i<=1;i++)
	{
		for(int j=-1;j<=1;j++)
		{
			s+=Cells[(j+y)*taille+x+i];
		}
	}
	s-=Cells[y*taille +x];
	return s; 
}

int main()
{
	srand(time(0));
	int cr=10;
	int cv=10;
	int cb=10;
    int taille=500;
    int hauteur=500;
	int pause=1;
	int host=1;
	int u=5;
	int* Cells=malloc(taille*taille*sizeof(int));
	int* Cellext=malloc(taille*taille*sizeof(int));
	for (int i=0;i<(taille*taille);i++)
	{
		Cellext[i]=0;
		Cells[i]=0;
	}
	SDL_Window *wdw;
	SDL_Renderer *rendu;
	if(SDL_Init(SDL_INIT_VIDEO))
		SDL_Exitwitherror("failed init");
	wdw = SDL_CreateWindow("Nom fenêtre",SDL_WINDOWPOS_CENTERED,0,hauteur,hauteur,0);			
	if(!wdw)
		SDL_Exitwitherror("window creation failed");
	rendu = SDL_CreateRenderer(wdw,-1,SDL_RENDERER_SOFTWARE);
	if(!rendu)
		SDL_Exitwitherror("renderer failed");
	SDL_bool Launched= SDL_TRUE;
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
				case(SDLK_p):
					pause=1-pause;
					break;
				case(SDLK_r):
					cr+=1;
					break;
				case(SDLK_g):
					cv+=1;
					break;
				case(SDLK_m):
						for (int i=taille;i<taille*taille/2;i++)
							{
								Cellext[i]=0;
								Cells[i]=0;
							}
					break;
				case(SDLK_l):
					host=1;
					break;
				case(SDLK_b):
					cb+=1;
					break;
				case(SDLK_h):
					host+=1;
					break;
				case(SDLK_y):
					if(host!=1)
						host-=1;
					break;
				case(SDLK_w):
					u+=1;
					break;
				case(SDLK_x):
					if(u!=1)
					u-=1;
				default:
					break;
				}
				break;
			    case SDL_MOUSEBUTTONDOWN:
                	int x=event.button.x;
                	int y=event.button.y;
                	if(Cells[taille*(y*taille/hauteur) +((taille*x)/hauteur)])
                	{
                    	Cells[taille*(y*taille/hauteur) +((taille*x)/hauteur)]=0;
                	}
                	else
                		Cells[taille*(y*taille/hauteur) +((taille*x)/hauteur)]=3;
					break;
			default:
				break;
			}
		}
		if(!pause)
		{	
			for(int l=1;l<(taille)-1;l++)
		{
			for (int k=1;k<taille-1;k++)
			{
				int t=CountN(Cells,k,l,taille);
				if(Cells[])
				if(t==3 )
				
			}
		}}
		for(int l=0;l<(taille);l++)
		{
			for (int k=0;k<taille;k++)
			{
				if(!pause){Cells[l*taille+k]=Cellext[l*taille +k];}
				int b=Cells[l*taille+k];
				SDL_SetRenderDrawColor(rendu,cr*b,cv*b,cb*b,255);
				SDL_RenderDrawPoint(rendu,k,l);
				
			}
		}
		SDL_RenderPresent(rendu);
		
}
/*************************************/
	free(Cells);
	free(Cellext);
	SDL_DestroyRenderer(rendu);
	SDL_DestroyWindow(wdw);
	SDL_Quit();
	return EXIT_SUCCESS;
}