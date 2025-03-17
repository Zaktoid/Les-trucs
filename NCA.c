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

float convoulute(float A[3][3],float* grid,int sizeK,int sizeG,int x,int y)
{
    float out=0;
    for(int a=0;a<sizeK;a++)
    {
        for(int b=0;b<sizeK;b++)
        {
            out+=-A[a][b]*grid[((x+a)%sizeG)*sizeG+(y+b)%sizeG];
        }
    }
    return out;
}



int main()
{
	srand(time(0));
	int cr=10;
	int cv=10;
	int cb=50;
    int hauteur=600;
	int res=10;
    int taille=hauteur/res;
	int pause=1;
	int u=1000;
	float* Cells=malloc(taille*taille*sizeof(float));
	float* Cellext=malloc(taille*taille*sizeof(float));
	for (int i=0;i<(taille*taille);i++)
	{
		Cellext[i]=0;
		Cells[i]=Cellext[i];
	}
	SDL_Window *wdw;
	SDL_Renderer *rendu;
    SDL_Window *wdw1;
	SDL_Renderer *rendu1;
	if(SDL_Init(SDL_INIT_VIDEO))
		SDL_Exitwitherror("failed init");
	wdw = SDL_CreateWindow("Nom fenêtre",SDL_WINDOWPOS_CENTERED,0,hauteur,hauteur,0);			
	if(!wdw)
		SDL_Exitwitherror("window creation failed");
	rendu = SDL_CreateRenderer(wdw,-1,SDL_RENDERER_SOFTWARE);
	if(!rendu)
		SDL_Exitwitherror("renderer failed");
	SDL_bool Launched= SDL_TRUE;
    int hauteur1=hauteur/2;
    wdw1 =SDL_CreateWindow("Nom fenêtre",SDL_WINDOWPOS_CENTERED,0,hauteur1,hauteur1,0);
	if(!wdw1)
		SDL_Exitwitherror("window creation failed");
    rendu1 =SDL_CreateRenderer(wdw1,-1,SDL_RENDERER_SOFTWARE);		
    float filter[3][3]={{0.1,-0.1,0.3},
                  {0,1,-0.3},
                  {-0.5,-0.1,0.2}};
    int sign=1;
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
				case(SDLK_m):
                    sign*=-1;
					break;
                case (SDLK_w):
                    Cells[(taille/2)*taille+taille/2]=1;
                    break;
                case(SDLK_ESCAPE):
                    Launched=SDL_FALSE;
                    break;
				default:
					break;
				}
				break;
			    case SDL_MOUSEBUTTONDOWN:
                	int x=event.button.x;
                	int y=event.button.y;
                    printf("%d,%d \n",x,y);
                    filter[(y*3)/hauteur1][(x*3)/hauteur1]+=0.1*sign;
					break;

			default:
				break;
			}
		}
		if(!pause)
		{	
			for(int l=0;l<(taille);l++)
		{

                for(int k=0;k<(taille);k++)
                {   
                    Cellext[l*taille+k]=convoulute(filter,Cells,3,taille,l,k);
                }
		}
		}
		SDL_Rect R;
		R.h=R.w=res;
		for(int l=0;l<(taille);l++)
		{
			for (int k=0;k<taille;k++)
			{
				if(!pause){Cells[l*taille+k]=Cellext[l*taille +k];}
				float b=Cells[l*taille+k];
				R.x=k*res;
				R.y=l*res;
				SDL_SetRenderDrawColor(rendu,0,cv*b,0,255);
				SDL_RenderFillRect(rendu,&R);
				
			}
		}
        R.h=R.w=(hauteur1/3);
        for(int i=0;i<3;i++)
            {
                for(int l=0;l<3;l++)
                {
                    float b=filter[i][l];
                    R.x=l*(hauteur1/3);
                    R.y=i*(hauteur1/3);
                    SDL_SetRenderDrawColor(rendu1,(b<0)*fabs(b)*50,0,50*fabs(b)*(b>0),255);
				    SDL_RenderFillRect(rendu1,&R);
                }

            }
        SDL_Delay(u);
		SDL_RenderPresent(rendu);
        SDL_RenderPresent(rendu1);
		
}
/*************************************/
	free(Cells);
	free(Cellext);
	SDL_DestroyRenderer(rendu);
	SDL_DestroyWindow(wdw);
    SDL_DestroyRenderer(rendu1);
	SDL_DestroyWindow(wdw1);
	SDL_Quit();
	return EXIT_SUCCESS;
}