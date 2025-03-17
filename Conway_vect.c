#include <stdlib.h>
#include <stdio.h>
#include <SDL.h>
#include <math.h>
#include <time.h>
#include "Mathutils.h"
void SDL_Exitwitherror(const char *message)
{
	SDL_Log("Erreur:%s >%s \n",message,SDL_GetError());
	exit(EXIT_FAILURE);
}

int main()
{
	srand(time(0));
	int cr=1;
	int cv=1;
	int cb=1;
    int taille=800;
    int hauteur=800;
	int pause=1;
	float trans=0.9;
	int u=1;
	complex_num* Cells=malloc(taille*taille*sizeof(complex_num));
	complex_num* Cellext=malloc(taille*taille*sizeof(complex_num));
	for (int i=0;i<(taille*taille);i++)
	{
		Cellext[i].Rz=0;
		Cells[i].Rz=0;
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
				case(SDLK_b):
					cb+=1;
					break;
				case(SDLK_h):
					trans+=1;
					break;
				case(SDLK_y):
					if(trans!=1)
						trans-=1;
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
					for(int k=-2;k<2;k++)
					{
						for(int l=-2;l<2;l++)
						{
							Cells[taille*((y+k)*taille/hauteur) +((taille*(x+l))/hauteur)].Rz+=4000;
							Cells[taille*((y+k)*taille/hauteur) +((taille*(x+l))/hauteur)].Iz+=4000;
						}
					}
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
				float s=0;
				float t=0;
				for(int j=-1;j<2;j++)
				{
					for(int i=-1;i<2;i++)
					{
						if(!(i==0 && j==0))
						{	
							s+=Cells[(l+i)*taille+k+j].Rz;
							t+=Cells[(l+i)*taille+k+j].Iz;
						}


					}
				}
				Cellext[l*taille+k].Rz=(1-trans)*Cells[l*taille+k].Rz+s*trans/8.0;
				Cellext[l*taille+k].Iz=(1-trans+Cellext[l*taille+k].Rz)*Cells[l*taille+k].Iz+t*trans/8.0;

                


			}
		}}
        if(!pause)
		{for(int l=0;l<(taille);l++)
		{
			for (int k=0;k<taille;k++)
			{
	            {Cells[l*taille+k]=Cellext[l*taille +k];}
				int b_x=Cells[l*taille+k].Rz;
				int b_y=Cells[l*taille+k].Rz;
				SDL_SetRenderDrawColor(rendu,cr*b_x,cv*b_y,0,255);
				SDL_RenderDrawPoint(rendu,k,l);
			}
		}}
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