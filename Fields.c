#include <stdlib.h>
#include <stdio.h>
#include <SDL2/SDL.h>
#include <math.h>
#include <time.h>
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
int Gravi(Object* A, Object* B, float G)
{
	float x_1=A->P[0];
	float x_2=B->P[0];
	float y_1=A->P[1];
	float y_2=B->P[1];
	float dist=(sqrt((x_1-x_2)* (x_1-x_2)+ (y_1-y_2)*(y_1-y_2)));
	if(dist==0)
	{
	return 1;
	}
	float G_x=-(x_1-x_2)/dist;
	float G_y=-(y_1-y_2)/dist;
	A->F_R[0]+=(A->m)*(B->m)*G_x/(G*dist*dist);
	A->F_R[1]+=(A->m)*(B->m)*G_y/(G*dist*dist);
	B->F_R[0]+=-(A->m)*(B->m)*G_x/(G*dist*dist);
	B->F_R[1]+=-(A->m)*(B->m)*G_y/(G*dist*dist);
	return 0;
}
void Collide_Box(Object* in,int x_min,int x_max,int y_min , int y_max,float absorb)
{
	if(in->P[0]>x_max || in->P[0]<x_min)
		{in->V[0]*=absorb;
		in->V[1]*=absorb;}
	if(in->P[1]>y_max || in->P[1]<y_min)
		{in->V[0]*=absorb;
		in->V[1]*=absorb;}
}
void Tor(Object* in,int x_min,int x_max,int y_min , int y_max)
{
	if(in->P[0]>x_max)
	{
		in->P[0]=x_min;
	}
	if(in->P[0]<x_min)
	{
		in->P[0]=x_max;
	}
	if(in->P[1]>y_max)
	{
		in->P[1]=y_min;
	}
	if(in->P[1]<y_min)
	{
		in->P[1]=y_max;
	}

}

void SDL_Exitwitherror(const char *message)
{
	SDL_Log("Erreur:%s >%s \n",message,SDL_GetError());
	exit(EXIT_FAILURE);
}

int main()
{
	float G=50000;
	srand(time(0));
    /**********intialisation d'élements SDL************/
	int hauteur=600;
	int Larg=600;
	SDL_Window *window;
	SDL_Renderer *rendu;
	if(SDL_Init(SDL_INIT_VIDEO))
		SDL_Exitwitherror("failed init");
	window= SDL_CreateWindow("Nom fenêtre",SDL_WINDOWPOS_CENTERED,0,hauteur,Larg,0);			
	if(!window)
		SDL_Exitwitherror("window creation failed");
	rendu = SDL_CreateRenderer(window,-1,SDL_RENDERER_ACCELERATED);
	if(!rendu)
		SDL_Exitwitherror("renderer failed");
	SDL_bool Launched= SDL_TRUE;
    /**********intialisation d'élements SDL************/
	int size=50;
	/***********************************************************/
	int TR=0;
    while (Launched)
	{
		SDL_SetRenderDrawColor(rendu,0,0,0,255);
		SDL_RenderClear(rendu);
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


				SDL_RenderPresent(rendu);
				TR++;
}
/*************************************/
	SDL_DestroyRenderer(rendu);
	SDL_DestroyWindow(window);
	SDL_Quit();
	return EXIT_SUCCESS;}

