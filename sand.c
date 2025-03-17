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
float CountN(float *Cells,int x , int y,int taille)
{
	float s=0;
	for(int i=-1;i<=1;i++)
	{
		for(int j=-1;j<=1;j++)
		{
			if(i||j)
				s+=Cells[(j+y)*taille+x+i];
		}
	}
	return s; 
}

int main()
{
	srand(time(0));
	int cr=10;
	int cv=10;
	int cb=100;
    int hauteur=400;
	int res=1;
    int taille=hauteur/res;
	int pause=1;
	int* Cells=malloc(taille*taille*sizeof(float));
	int* Cellext=malloc(taille*taille*sizeof(float));
	for (int i=0;i<(taille*taille);i++)
	{
		Cellext[i]=0;
		Cells[i]=Cellext[i];
	}
	SDL_Window *wdw;
	SDL_Renderer *rendu;
	SDL_Rect R;
	R.h=R.w=res;
	if(SDL_Init(SDL_INIT_VIDEO))
		SDL_Exitwitherror("failed init");
	wdw = SDL_CreateWindow("Nom fenêtre",SDL_WINDOWPOS_CENTERED,0,hauteur,hauteur,0);			
	if(!wdw)
		SDL_Exitwitherror("window creation failed");
	rendu = SDL_CreateRenderer(wdw,-1,SDL_RENDERER_SOFTWARE);
	if(!rendu)
		SDL_Exitwitherror("renderer failed");
	SDL_bool Launched= SDL_TRUE;
	int J=1;
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
				case(SDLK_b):
					cb+=1;
					break;
				default:
					break;
				}
				break;
			    case SDL_MOUSEBUTTONDOWN:
                	int x=event.button.x;
                	int y=event.button.y;

                    	Cellext[taille*(y*taille/hauteur) +((taille*x)/hauteur)]+=100000000;
					break;
			default:
				break;
			}
		}
		if(!pause)
		{	
			for(int l=J;l<(taille)-J;l++)
		{
			for (int k=J;k<taille-J;k++)
			{
				if(Cells[l*taille+k]>=(4*(J+1)*J/2)+(J/4+1)*J/2)
                {
                    Cellext[l*taille+k]-=4*(J+1)*J/2+(J/4+1)*J/2;
					for(int i=1;i<=J;i++)
					{
						Cellext[(l+i)*taille+k]+=i;
						Cellext[(l-i)*taille+k]+=i;
						Cellext[l*taille+k+i]+=i;
						Cellext[l*taille+k-i]+=i;
					}
					for(int i=1;i<=(J/4);i++)
					{
						Cellext[(l+i)*taille+k+i]+=i;
						Cellext[(l-i)*taille+k-i]+=i;
						Cellext[(l-i)*taille+k+i]+=i;
						Cellext[(l+i)*taille+k-i]+=i;
					}

                }

			}
		}
		}
		for(int l=0;l<(taille);l++)
		{
			for (int k=0;k<taille;k++)
			{
				if(!pause){Cells[l*taille+k]=Cellext[l*taille +k];}
				float b=Cells[l*taille+k];
				R.x=k*res;
				R.y=l*res;
				SDL_SetRenderDrawColor(rendu,cr*b,cv*b,cb*b,255);
				SDL_RenderFillRect(rendu,&R);
				
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