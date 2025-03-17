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
    int hauteur=200;
	int res=1;
    int taille=hauteur/res;
	int pause=1;
	int u=10;
	int*** Cells=malloc(taille*sizeof(int**));
	int*** Cellext=malloc(taille*sizeof(int**));
	for (int i=0;i<taille;i++)
	{
        Cells[i]=malloc(taille*sizeof(int**));
        Cellext[i]=malloc(taille*sizeof(int**));
        for(int j=0;j<taille;j++)
        {
            Cells[i][j]=malloc(taille*sizeof(int*));
            Cellext[i][j]=malloc(taille*sizeof(int*));
            for(int k=0;k<taille;k++)
            {
                Cells[i][j][k]=0;
            }
        }

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
    int C=(taille/2);
    printf("%d \n",C);
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
				case(SDLK_w):
					u+=1;
					break;
				case(SDLK_x):
                    C+=1;
                    break;
                case(SDLK_d):
                    C-=1;
                    break;
				default:
					break;
				}
				break;
			    case SDL_MOUSEBUTTONDOWN:
                	int x=event.button.x;
                	int y=event.button.y;

                    	Cellext[C][(taille/2)][(taille/2)]+=1000000;
					break;
			default:
				break;
			}
		}
		if(!pause)
		{
        for(int i=taille/2-5;i<taille/2+5;i++)
        {	
			for(int l=1;l<(taille)-1;l++)
		    {
			for (int k=1;k<taille-1;k++)
			{    
                    if(Cells[i][l][k]>=6)
                    {
                        Cellext[i][l][k]-=6;
                        Cellext[i][l][k+1]+=1;
                        Cellext[i][l][k-1]+=1;
                        Cellext[i][l+1][k]+=1;
                        Cellext[i][l-1][k]+=1;
                        Cellext[i+1][l][k]+=1;
                        Cellext[i-1][l][k]+=1;
                    }
                
			}
		    }
		}
        }
		for(int i=taille/2-5;i<taille/2+5;i++)
		{
			for (int l=0;l<taille;l++)
			{
                for(int k=0;k<taille;k++)
 
                   { if(!pause)
                    {
                        Cells[i][l][k]=Cellext[i][l][k];
                    }
				    int b=Cellext[C][l][k];
				    R.x=k*res;
				    R.y=l*res;
				    SDL_SetRenderDrawColor(rendu,cr*b,cv*b,cb*b,255);
				    SDL_RenderFillRect(rendu,&R);
                    }

				
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