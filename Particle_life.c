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
	int color;
	
}Particle;

Particle* Creat_Obj(float* P)
{
	Particle* out=malloc(sizeof(Particle));
	if(!out){return NULL;}
	out->P=P;
    out->V=malloc(sizeof(float)*2);
    out->F_R=malloc(sizeof(float)*2);
    out->F_R[0]=out->F_R[1]=0;
    return out;
}

void Upgrade(Particle in,float TimeStep)
{
	float v0_x=in.V[0];
	float v0_y=in.V[1];
	float d_vx=(in.F_R[0])/TimeStep;
	float d_vy=(in.F_R[1])/TimeStep;
	in.V[0]+=d_vx;
	in.V[1]+=d_vy;
	in.P[0]+=v0_x/TimeStep;
	in.P[1]+=v0_y/TimeStep;


}
int Interact(Particle* A, Particle* B, float r_min,float r_max,float** Tab,float Repulse)
{
	float x_1=A->P[0];
	float x_2=B->P[0];
	float y_1=A->P[1];
	float y_2=B->P[1];
	float dist=(sqrt((x_1-x_2)* (x_1-x_2)+ (y_1-y_2)*(y_1-y_2)));
	float G_x=(x_2-x_1)/dist;
	float G_y=(y_2-y_1)/dist;
    if(dist==0 || dist>=r_max)
    {
        return 1;
    }
    float C_1=2.0*Tab[A->color][B->color]/(r_max-r_min);
    float C_2=2.0*Tab[B->color][A->color]/(r_max-r_min);
    if(dist<r_min)
    {
        A->F_R[0]+=dist*(-Repulse+(Repulse/r_min))*G_x;
        A->F_R[1]+=dist*(-Repulse+(Repulse/r_min))*G_y;
        B->F_R[0]+=-dist*(-Repulse+(Repulse/r_min))*G_x;
	    B->F_R[1]+=-dist*(-Repulse+(Repulse/r_min))*G_y;
    }
    if(r_min<dist && dist<(r_min+r_max)/2.0)
    {
        A->F_R[0]=dist*(C_1)*G_x;
        A->F_R[1]=dist*(C_1)*G_y;
        B->F_R[0]+=-dist*(C_2)*G_x;
	    B->F_R[1]+=-dist*(C_2)*G_y; 
    }
    if(dist<(r_min+r_max)/2.0 && dist<r_max)
    {
        A->F_R[0]=(-C_1)*G_x;
        A->F_R[1]=(-C_1)*G_y;
        B->F_R[0]+=-(-C_2)*G_x;
	    B->F_R[1]+=-(-C_2)*G_y; 
    }
	return 0;
}


void SDL_Exitwitherror(const char *message)
{
	SDL_Log("Erreur:%s >%s \n",message,SDL_GetError());
	exit(EXIT_FAILURE);
}

int main()
{
	srand(time(0));
    /**********Variables simulation************/
	int hauteur=800;
	int Larg=800;
    float r_min=5.0;
    float r_max=300.0;
    int nb_colors=3;
    ////////////////////Interaction Matrix///////////////////
    float** Tab=malloc(sizeof(float*)*nb_colors);
    for(int j=0;j<nb_colors;j++)
    {
        Tab[j]=malloc(sizeof(float)*nb_colors);
    }
    for(int l=0;l<nb_colors;l++)
    {
        for(int k=0;k<nb_colors;k++)
            {Tab[l][k]=-2+4*(float)(rand()%100)/100.0;
            }


    }
    ////////////////////////////Initialisation///////////////////////////////
	SDL_Window *window;
	SDL_Renderer *rendu;
	if(SDL_Init(SDL_INIT_VIDEO))
		SDL_Exitwitherror("failed init");
	window= SDL_CreateWindow("Nom fenêtre",SDL_WINDOWPOS_CENTERED,0,hauteur,Larg,0);			
	if(!window)
		SDL_Exitwitherror("window creation failed");
	rendu = SDL_CreateRenderer(window,-1,SDL_RENDERER_SOFTWARE);
	if(!rendu)
		SDL_Exitwitherror("renderer failed");
	SDL_bool Launched= SDL_TRUE;
    ////////////////////////////////Sims elements/////////////////////////////////////////////
	int nb=800;
	Particle** Things=malloc(nb*sizeof(Particle*));
	for(int k=0;k<nb;k++)
	{
		float* P=malloc(2*sizeof(float));
		P[0]=rand()%hauteur;
		P[1]=rand()%hauteur;
		Things[k]=Creat_Obj(P);
		Things[k]->V[0]=0;
		Things[k]->color=rand()%nb_colors;
		Things[k]->V[1]=0;
	}
	/***********************************************************/
	int TR=0;
    while (Launched)
	{
		SDL_SetRenderDrawColor(rendu,30,10,10,255);
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
		for(int i=0;i<nb;i++)
			{
				for(int j=(i+1)%nb;j!=i;j=(j+1)%nb)
				{
					Interact(Things[i],Things[j],r_min,r_max,Tab,10);

						/****************************************************/
				}
			}
				for(int k=0;k<nb;k++)
				{
					Upgrade(*Things[k],100);
					Things[k]->V[0]*=0.993;
					Things[k]->V[1]*=0.993;
					Things[k]->F_R[0]=0;
					Things[k]->F_R[1]=0;
					float C=Things[k]->color;
                    SDL_SetRenderDrawColor(rendu,255*(C==1),255*(C==0),255*(C==2),255);
                    SDL_RenderDrawPoint(rendu,Things[k]->P[0],Things[k]->P[1]);	
				}
				SDL_RenderPresent(rendu);
				TR++;
}
/*************************************/
	SDL_DestroyRenderer(rendu);
	SDL_DestroyWindow(window);
	for(int k=0;k<nb;k++)
	{
		free(Things[k]->P);
		free(Things[k]->V);
		free(Things[k]);
	}
	free(Things);
    free(Tab);
	SDL_Quit();
	return EXIT_SUCCESS;}

