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

typedef struct Genom
{
    int Genes[16][5];
}Genom;

typedef struct Organism
{
    complex_num P;
    Genom G;
    int* percept;
    int* Action;
    
}Organism;

float** Decod(Genom* I,int Ninput,int Nhidden,int Nouput)
{
    float** Wiring=malloc(sizeof(float*)*(Ninput+Nhidden));
    for(int i=0;i<(Nhidden+Ninput);i++)
        {
            Wiring[i]=malloc((Nouput+Ninput)*sizeof(float));
            for(int k=0;k<Nouput+Ninput;k++)
            {
                Wiring[i][k]=0;
            }
        }
    for(int i=0;i<16;i++)
    {
        if(I->Genes[i][0]==0)
        {

            if(I->Genes[i][3]==0)
            {
                Wiring[Ninput+(I->Genes[i][1])%Nhidden][I->Genes[i][4]%Nhidden]=((float)I->Genes[i][4])/100.0;
            }
            else
            {
                Wiring[Ninput+(I->Genes[i][1])%Nhidden][Nhidden+I->Genes[i][4]%Nouput]=((float)I->Genes[i][4])/100.0;
            }
        }    
        else
        {
            if(I->Genes[i][3]==0)
            {
                Wiring[(I->Genes[i][1])%Ninput][I->Genes[i][4]%Nhidden]=(float)I->Genes[i][2]/100;
            }
            else
            {
                Wiring[(I->Genes[i][1])%Ninput][Nhidden+I->Genes[i][4]%Nouput]=(float)I->Genes[i][2]/100;
            }

        }  
    }
    return Wiring;
}

float* Update(Organism In)
{
    for(int i=0;i<16;i++)
    {
        if(In.G.Genes[i][0]==0)
        {

        }

    }
}


int main(int argc , char **argv)
{
	srand(time(NULL));
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
    int Ninput=10;
    int Nhidden=3;
    int Nouput=5;
    /*
    Ins:
    0 Hunger
    1 Random input
    2 Blockage LeftRight
    3 Oscillator
    4 Food proximitity 1 if in the hood, 0 otherwise
    5 x position (0:0 , 1:hauteur) 
    6 y position (0:0 , 1:hauteur)
    7 last movement x
    8 last movement y
    9 distance to the edge min(x,y);
    Outs:
    0 Move x
    1 Kill
    2 Move y
    3 Eat
    4 Move random
    5 Move Diag++
    6 Move Diag-+
    7 Move Diag +-
    8 Move Diag -- 
    */
    Genom G;
    for(int i=0;i<16;i++)
    {
        G.Genes[i][0]=rand()%2;
        G.Genes[i][1]=(rand()%Ninput)*(G.Genes[i][0]==1)+(rand()%Nhidden)*(G.Genes[i][0]==0);
        G.Genes[i][2]=rand()%200-100;
        G.Genes[i][3]=rand()%2;
        G.Genes[i][4]=(rand()%Nouput)*(G.Genes[i][3]==1)+(rand()%Nhidden)*(G.Genes[i][3]==0);
    }
    for(int i=0;i<16;i++)
    {
        printf(" gene#: %d, %d.%d.%d.%d.%d \n",i,G.Genes[i][0],G.Genes[i][1],G.Genes[i][2],G.Genes[i][3],G.Genes[i][4]);
    }
    Organism A;
    A.percept=malloc(sizeof(Ninput));
    A.P.Iz=hauteur/2;
    A.P.Rz=hauteur/2;
    A.G=G;
    A.Action=malloc(sizeof(Nouput));
    float** Wiring=Decod(&G,Ninput,Nhidden);
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

	SDL_DestroyRenderer(rendu);
	SDL_DestroyWindow(window);
	SDL_Quit();
	return EXIT_SUCCESS;

}