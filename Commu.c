#include <stdlib.h>
#include <stdio.h>
#include <SDL2/SDL.h>
#include <math.h>
#include <time.h>


void SDL_Exitwitherror(const char *message)
{
	SDL_Log("Erreur:%s >%s \n",message,SDL_GetError());
	exit(EXIT_FAILURE);
}
int count_n(int* Array,int size,int x,int y,int b)
{
    int s=0;
    for(int l=-1;l<2;l++)
    {
        for(int k=-1;k<2;k++)
        {
            if(Array[(x+l)*size+y+k]==b)
                s+=1;
        }
    }
    return s-1;
}


int main()
{
	srand(time(NULL));
    /**********intialisation d'élements SDL************/
	int hauteur=700;
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
    int* Array=malloc(sizeof(int)*hauteur*hauteur);
	int P=3;
    for(int k=0;k<hauteur*hauteur;k++)
    {
        Array[k]=rand()%2;
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
			case SDL_KEYDOWN:
				switch (event.key.keysym.sym)
				{
				case SDLK_a:
					P++;
					break;
				case SDLK_z:
					P--;
					break;
				default:
					break;
				}
			default:
				break;
		}}
		for(int k=1;k<hauteur-1;k++)
		{
			for(int l=1;l<hauteur-1;l++)
			{
                if(count_n(Array,hauteur,k,l,Array[k*hauteur+l])<=P)
                {

                    int s_1=rand()%hauteur;
                    int s_2=rand()%hauteur;
					int a=(s_1)*hauteur +s_2;
                    int u=Array[k*hauteur+l];
					Array[k*hauteur+l]=Array[a];
                    Array[a]=u;

                }
			}
		}

        for(int k=0;k<hauteur;k++)
        {
            for(int l=0;l<hauteur;l++)
            {
                int s=Array[k*hauteur+l];
                SDL_SetRenderDrawColor(rendu,(s==0)*255,(s==1)*255,(s==2)*255,255);
                SDL_RenderDrawPoint(rendu,k,l);
            }
        }
        SDL_RenderPresent(rendu);
        
		
}
/*************************************/
	SDL_DestroyRenderer(rendu);
	SDL_DestroyWindow(window);
    free(Array);
	SDL_Quit();
	return EXIT_SUCCESS;

}