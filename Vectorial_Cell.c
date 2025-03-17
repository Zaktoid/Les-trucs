#include <stdlib.h>
#include <stdio.h>
#include <SDL.h>
#include <math.h>
#include <time.h>

void SDL_Exitwitherror(const char *message)
{
	SDL_Log("Erreur:%s >%s \n",message,SDL_GetError());
	exit(EXIT_FAILURE);
}
typedef struct
{
    int v[2];
}cell;


int main(int argc , char **argv)
{
	int hauteur=400;
	int taille=200;
    cell* Cellules=malloc(taille*taille*sizeof(cell));
	cell* Cellext=malloc(taille*taille*sizeof(cell));
    int pause=1;
    float transmi=1;
    srand(time(0));
	int hostilite=1;
	for (int i=0;i<taille*taille;i++ )
	{
		Cellules[i].v[0]=0;
		Cellules[i].v[1]=0;
		Cellext[i].v[0]=0;
		Cellext[i].v[1]=0;
	}
    
	SDL_Window *wdw;
	SDL_Renderer *rendu;
    /**********intialisation d'élements SDL************/
	if(SDL_Init(SDL_INIT_VIDEO))
		SDL_Exitwitherror("Echec init");
	wdw = SDL_CreateWindow("STP fonctionne :( ",SDL_WINDOWPOS_CENTERED,0,hauteur,hauteur,0);			
	if(!wdw)
		SDL_Exitwitherror("Creation fenêtre échouée");
	rendu = SDL_CreateRenderer(wdw,-1,SDL_RENDERER_ACCELERATED);
	if(!rendu)
		SDL_Exitwitherror("Echec Rendu");
    if(SDL_SetRenderDrawColor(rendu,255,255,255,255))
		SDL_Exitwitherror("Echec pinceau");
	SDL_bool Launched= SDL_TRUE;
    /************inputs***********************/
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
                    case (SDLK_h):
                        transmi=transmi+0.05;
                        break;
                    case(SDLK_j):
						if(transmi -0.5>0)
                        transmi=transmi-0.05;
						break;
					case(SDLK_m):
						for (int i=taille;i<taille*taille/2;i++)
							{
								Cellext[i].v[0]=0;
								Cellext[i].v[1]=0;
								Cellules[i].v[0]=0;
								Cellules[i].v[1]=0;
							}
					break;
                    default:
                        break;
                }
                    break;
            case SDL_MOUSEBUTTONDOWN:
			        int x=event.button.x;
                	int y=event.button.y;
			        if(!Cellules[taille*(y*taille/hauteur) +((taille*x)/hauteur)].v[0])
                	{
                    	Cellules[taille*(y*taille/hauteur) +((taille*x)/hauteur)].v[0]+=10;
						Cellules[taille*(y*taille/hauteur) +((taille*x)/hauteur)].v[1]+=10;
                	}
                	else
                		Cellules[taille*(y*taille/hauteur) +((taille*x)/hauteur)].v[0]=0;
						Cellules[taille*(y*taille/hauteur) +((taille*x)/hauteur)].v[1]=0;
					break;
			default:
				break;
		}}
	/*******************************************/
	if(!pause)
	{
		for(int k=1;k<(taille-1);k++)
		{
			for(int l=1;l<(taille-1);l++)
			{
				int s[2];
				s[0]=0;
				s[1]=0;
				for(int i=-1;i<=1;i++)
				{
					for(int j=-1;j<=1;j++)
					{
						if(i!=j)
					{s[0]+=transmi*Cellules[(j+l)*taille+k+i].v[1];
					s[1]+=transmi*Cellules[(j+l)*taille+k+i].v[0];
					}

					}
				}
				Cellext[l*taille+k].v[0]=s[0];
				Cellext[l*taille+k].v[1]=s[1];
			}
		}
	}
				for(int l=0;l<(taille);l++)
			{
				for (int k=0;k<taille;k++)
				{
					
					if(!pause){Cellules[k*taille+l]=Cellext[k*taille +l];}
					SDL_Rect rectangle={l*(hauteur/taille),(hauteur/taille)*(k),hauteur/taille,hauteur/taille};
					SDL_SetRenderDrawColor(rendu,Cellules[k*taille+l].v[0],Cellules[k*taille+l].v[1],0,255);
					SDL_RenderFillRect(rendu, &rectangle);
					
					
				}
			}	
		SDL_RenderPresent(rendu);
		}
    /****************************************/

/*************************************/
	free(Cellules);
	free(Cellext);
	SDL_DestroyRenderer(rendu);
	SDL_DestroyWindow(wdw);
	SDL_Quit();
	return EXIT_SUCCESS;
}

