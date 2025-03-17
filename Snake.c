#include <stdlib.h>
#include <stdio.h>
#include <SDL2/SDL.h>
#include <math.h>
void SDL_Exitwitherror(const char *message)
{
	SDL_Log("Erreur:%s >%s \n",message,SDL_GetError());
	exit(EXIT_FAILURE);
}
typedef struct 
{
	float* P;
	float* V;
	float* F_R;
	float m;
	
}Object;

Object* Creat_Obj(float* P)
{
	Object* out=malloc(sizeof(Object));
	if(!out){return NULL;}
	out->P=P;
    out->V=malloc(sizeof(int)*2);
    out->F_R=malloc(sizeof(int)*2);
    out->F_R[0]=out->F_R[1]=0;
    return out;
}

void Upgrade(Object in)
{
	float v0_x=in.V[0];
	float v0_y=in.V[1];
	float d_vx=in.F_R[0]/in.m;
	float d_vy=in.F_R[1]/in.m;
	in.V[0]+=d_vx;
	in.V[1]+=d_vy;
	in.P[0]+=v0_x;
	in.P[1]+=v0_y;


}



int main(int argc , char **argv)
{
    /**********intialisation d'élements SDL************/
	int hauteur=800;
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
				default:
					break;
				}
			default:
				break;
		}}
		
}
/*************************************/
	/*SDL_DestroyRenderer(rendu);
	SDL_DestroyWindow(window);
	SDL_Quit();
	return EXIT_SUCCESS;*/

}