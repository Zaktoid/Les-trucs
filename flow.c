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
complex_num speedtransm(complex_num* Array,int size,int x,int y,float tr)
{
    complex_num s={0,0};
    complex_num D={tr,0};
    for(int l=-1;l<2;l++)
    {
        for(int k=-1;k<2;k++)
        {
            if((k||l)&&(x+k)<size&&(y+l)<size
            &&(x+k>=0)&&(y+l>=0))
            {
                s=sum(Array[(x+k)*size+y+l],s);
            }
        }
    }
    return prod(s,D);
}


int main()
{
	srand(time(NULL));
    /**********intialisation d'élements SDL************/
	int hauteur=400;
	SDL_Window *window;
	SDL_Renderer *rendu;
	if(SDL_Init(SDL_INIT_VIDEO))
		SDL_Exitwitherror("failed init");
	window= SDL_CreateWindow("Nom fenêtre",SDL_WINDOWPOS_CENTERED,0,hauteur,hauteur,0);			
	if(!window)
		SDL_Exitwitherror("window creation failed");
	rendu = SDL_CreateRenderer(window,-1,SDL_RENDERER_ACCELERATED);
	if(!rendu)
		SDL_Exitwitherror("renderer failed");
	SDL_bool Launched= SDL_TRUE;
    complex_num* Array=malloc(sizeof(complex_num)*hauteur*hauteur);
    complex_num* Array_n=malloc(sizeof(complex_num)*hauteur*hauteur);
    float range=50;
    complex_num C={hauteur/2,hauteur/2};
    int size=4000;
    float arrow=5;
    for(int k=0;k<hauteur;k++)
    {
        for(int l=0;l<hauteur;l++)
        {
            complex_num T={k,l};
            if(Dist(T,C)>0)
            {
                float s=2*range*((float)rand()/(float)RAND_MAX)-range;
                float t=2*range*((float)rand()/(float)RAND_MAX)-range;
                Array[k*hauteur+l].Iz=t;
                Array[k*hauteur+l].Rz=s;
                Array_n[k*hauteur+l].Iz=Array[k*hauteur+l].Iz;
                Array_n[k*hauteur+l].Rz=Array_n[k*hauteur+l].Rz;
            }
        }
    }
    complex_num *A=malloc(size*sizeof(complex_num));
    for(int k=0;k<size;k++)
    {
        A[k].Rz=rand()%hauteur;
        A[k].Iz=rand()%hauteur;
    }
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
            case SDL_MOUSEBUTTONDOWN:
                	int x=event.button.x;
                	int y=event.button.y;
                	Array[x*hauteur +y].Rz+=10;
                    Array[x*hauteur +y].Iz+=10;
                    break;
			case SDL_KEYDOWN:
				switch (event.key.keysym.sym)
				{
                case SDLK_a:
                    arrow*=2;
                    break;
                case SDLK_z:
                    arrow/=2;
                    break;
				default:
					break;
				}
			default:
				break;
		}}
		for(int k=0;k<hauteur;k++)
		{
			for(int l=0;l<hauteur;l++)
			{
                Array_n[k*hauteur+l]=speedtransm(Array,hauteur,k,l,0.125);
			}
		}

        for(int k=0;k<hauteur;k++)
        {
            for(int l=0;l<hauteur;l++)
            {
                Array[k*hauteur+l]=Array_n[k*hauteur+l];
                float s=module(Array[k*hauteur+l]);
                SDL_SetRenderDrawColor(rendu,s*10*255,s*250,s*240,255);
                SDL_RenderDrawPoint(rendu,k,l);
                SDL_SetRenderDrawColor(rendu,255,255,255,255);
               if(!(k%5)&&!(l%5))
                   {
                    SDL_RenderDrawLine(rendu,k,l,k+arrow*Array[k*hauteur+l].Rz,l+arrow*Array[k*hauteur+l].Iz);
                    }
            }
        }
        SDL_SetRenderDrawColor(rendu,0,255,0,255);
        for(int k=0;k<size;k++)
        {
            if(((int)A[k].Rz*hauteur+(int)A[k].Iz)<hauteur*hauteur)
            {   
                float v_x=Array[(int)A[k].Rz*hauteur+(int)A[k].Iz].Rz;
                float v_y=Array[(int)A[k].Rz*hauteur+(int)A[k].Iz].Iz;
                if((A[k].Rz+v_x>=0)&&(A[k].Rz+v_x)<=hauteur)
                    A[k].Rz+=v_x;
                if((A[k].Iz+v_y>=0)&&(A[k].Iz+v_y)<=hauteur)
                A[k].Iz+=v_y;
            }

            SDL_RenderDrawPoint(rendu,A[k].Rz,A[k].Iz);

        }
        SDL_RenderPresent(rendu);
        SDL_SetRenderDrawColor(rendu,0,0,0,255);
        SDL_RenderClear(rendu);
        
		
}
/*************************************/
	SDL_DestroyRenderer(rendu);
	SDL_DestroyWindow(window);
    free(Array);
    free(A);
	SDL_Quit();
	return EXIT_SUCCESS;

}