#include <stdlib.h>
#include <stdio.h>
#include <SDL2/SDL.h>
#include <math.h>

typedef struct
{
	float x;
	float y;
	float z;
	int* Color;
}Voxel;

Voxel* Creat3Dseg(Voxel A,Voxel B,int prec)
{
	Voxel* out=malloc(sizeof(Voxel)*prec);
	if(!out)return NULL;
	for(int k=0;k<prec;k++)
	{
		out[k].x=(float)k/prec*A.x +(1-k/prec)*B.x;
		out[k].y=(float)k/prec*A.y +(1-k/prec)*B.y;
		out[k].z=(float)k/prec*A.z +(1-k/prec)*B.z;

	}
	return out;
}

void SDL_Exitwitherror(const char *message)
{
	SDL_Log("Erreur:%s >%s \n",message,SDL_GetError());
	exit(EXIT_FAILURE);
}
int main()
{
    /**********intialisation d'élements SDL************/
	Voxel A;
	A.x=100;
	A.y=100;
	A.Color=malloc(sizeof(int)*3);
	A.Color[0]=A.Color[1]=0;
	A.Color[2]=255;
	Voxel B=A;
	B.x=0;
	B.y=0;
	Voxel C=B;
	C.x=A;
	int hauteur=400;
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
	SDL_SetRenderDrawColor(rendu,A.Color[0],A.Color[1],A.Color[2],0);
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
				case SDLK_z:
					A.z+=1;
					break;
				case SDLK_y:
					A.y+=1;
					break;
				default:
					break;
				}
			default:
				break;
		}}
		Voxel* Seg=Creat3Dseg(A,B,50);
		for(int k=0;k<50;k++)
		{
		SDL_SetRenderDrawColor(rendu,200,atanh(Seg[k].z)*205,50,255);
		SDL_RenderDrawPoint(rendu,Seg[k].x,Seg[k].y);
		}
		SDL_RenderPresent(rendu);
		SDL_SetRenderDrawColor(rendu,0,0,0,255);
		SDL_RenderClear(rendu);
}
/*************************************/
	SDL_DestroyRenderer(rendu);
	SDL_DestroyWindow(window);
	SDL_Quit();
	return EXIT_SUCCESS;

}