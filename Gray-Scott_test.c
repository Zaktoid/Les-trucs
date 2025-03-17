#include <stdlib.h>
#include <stdio.h>
#include <SDL2/SDL.h>
#include <math.h>
#include "Mathutils.h"
void SDL_Exitwitherror(const char *message)
{
	SDL_Log("Erreur:%s >%s \n",message,SDL_GetError());
	exit(EXIT_FAILURE);

}

typedef struct Cell
{
    float C_A;
    float C_B;
}Cell;

float CountN(Cell *Cells,int x , int y,int taille, int B)
{
	float s=0;
	for(int i=-1;i<=1;i++)
	{
		for(int j=-1;j<=1;j++)
		{
            if(i||j)
                s+=Cells[(x+i)*taille+y+j].C_A*B*0.125+ Cells[(x+i)*taille+y+j].C_B*(1-B)*0.125;
            else
                s+=-1*Cells[(x+i)*taille+y+j].C_A*B+ -1*Cells[(x+i)*taille+y+j].C_B*(1-B);
		}
	}
	return s;
}



int main()
{
    /********variables principales*****/
    unsigned B=1;
    int hauteur=400;
    int taille=400;
    Cell *cases=malloc(taille*taille*sizeof(Cell));
    Cell *cases_s=malloc(taille*taille*sizeof(Cell));
    int pause=0;
    float T=1;
    float r_a=1;
    float r_b=0.5;
    float rate=0.097;
    float apporov=0.034;
    float C_R=1;
    float C_G=1;
    float C_B=1;
    slider* S=malloc(sizeof(slider)*4);
    S[0].actu=1; //r_a
    S[1].actu=0.5;  //r_b
    S[2].actu=0.095; //r
    S[3].actu=0.034; //k
    for(int k=0;k<4;k++)
    {
        S[k].inf=0;
        S[k].sup=1;
        S[k].length=100;
        S[k].X=0;
        S[k].Y=k*hauteur/4.0;
        S[k].targeted=0;
    }
    S[2].sup=0.3;
    S[3].sup=0.3;
    for (int i=0;i<taille*taille;i++ )
	{ 
        cases[i].C_A=0;
        cases[i].C_B=((float)(rand()%101)/100.0);
        cases_s[i].C_A=  cases[i].C_B;
        cases_s[i].C_B=0;
    }
    SDL_Window *wdw;
	SDL_Renderer *rendu;
	SDL_Texture *text;
	if(SDL_Init(SDL_INIT_VIDEO))
		SDL_Exitwitherror("failed init");
	wdw = SDL_CreateWindow("Gray Scott",SDL_WINDOWPOS_CENTERED,0,hauteur,hauteur,0);			
	if(!wdw)
		SDL_Exitwitherror("window creation failed");
	rendu = SDL_CreateRenderer(wdw,-1,SDL_RENDERER_SOFTWARE);
	if(!rendu)
		SDL_Exitwitherror("renderer failed");
	SDL_bool Launched= SDL_TRUE;
    int t=0;
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
                t=1;
                int x=event.button.x;
                int y=event.button.y;
                for(int k=0;k<4;k++)
                {
                    if((x>=S[k].X && x<=S[k].X+S[k].length)&& (y<=S[k].Y+5 && y>=S[k].Y-5))
                    {
                        t=0;
                        S[k].targeted=1;
                    }
                }
                break;
                break;
            case SDL_MOUSEMOTION:
                if(t)
                {   
                    int x=event.button.x;
                    int y=event.button.y;
                    for(int i=-4;i<=4;i++)
                    {
                        for(int l=-4;l<=4;l++)
                        {
                            cases[taille*((y+i)*taille/hauteur)+(taille*(x+l)/hauteur)].C_B+=0.02*B;
                            if(cases[taille*((y+i)*taille/hauteur)+(taille*(x+l)/hauteur)].C_B>=1)
                                cases[taille*((y+i)*taille/hauteur)+(taille*(x+l)/hauteur)].C_B=0.5;
                            cases[taille*((y+i)*taille/hauteur)+(taille*(x+l)/hauteur)].C_A+=(1-B);

                        }
                    }
                }
                for(int k=0;k<4;k++)
                    {
                        if(S[k].targeted)
                        {
                            float r=S[k].sup*(event.button.x-S[k].X)/S[k].length +S[k].inf;
                            if(r>=S[k].inf && r<=S[k].sup)
                            {
                                S[k].actu=r;
                            }
                        }
                    }
                break;                
                break;
            case SDL_MOUSEBUTTONUP:
                t=0;
                for(int k=0;k<4;k++)
                    {
                        S[k].targeted=0;
                    }
                break;
            case SDL_KEYDOWN:
                switch (event.key.keysym.sym)
                {
                    case(SDLK_p):
                        pause=1-pause;
                        break;
                    case(SDLK_d):
                        if(r_a-0.01>0)
                        r_a-=0.01;
                        break;
                    case(SDLK_e):
                        r_a+=0.01;
                        break;
                    case(SDLK_a):
                        B=1-B;
                        break;
                    case (SDLK_c):
                        C_R+=0.1;
                        break;
                    case (SDLK_v):
                        C_G+=0.1;
                        break;
                    case (SDLK_b):
                        C_B+=0.1;
                        break;
                    default:
                        break;
                }
			default:
				break;
			}
        }
    if(!pause)
	{
		for(int l=1;l<(taille-1);l++)
		{
			for(int k=1;k<(taille-1);k++)
			{
                float A=cases[l*taille+k].C_A;
                float B=cases[l*taille+k].C_B;
                cases_s[l*taille+k].C_A+= (S[0].actu*CountN(cases,l,k,taille,1)-A*B*B +S[3].actu*(1-cases[l*taille+k].C_A))/T;
                cases_s[l*taille+k].C_B+=(S[1].actu*CountN(cases,l,k,taille,0)+A*B*B-(S[2].actu)*cases[l*taille+k].C_B)/T;

            }
        }
    }

		    
	    
		for(int l=0;l<(taille);l++)
			{
				for (int k=0;k<taille;k++)
				{
					if(!pause)
                    {
                        cases[l*taille+k]=cases_s[l*taille +k];
                    }
					SDL_Rect rectangle={k*(hauteur/taille),(hauteur/taille)*(l),hauteur/taille,hauteur/taille};
                    float A=cases[l*taille+k].C_A;
                    float B=cases[l*taille+k].C_B;
                    SDL_SetRenderDrawColor(rendu,B*C_R*30,A*C_G*30,(A/(A+B))*C_B*100,255);
					SDL_RenderFillRect(rendu, &rectangle);
					
					
				}
			}
            for(int k=0;k<4;k++)
                Draw_slider(S[k],rendu);
            SDL_RenderPresent(rendu);
            SDL_SetRenderDrawColor(rendu,0,0,0,255);
            SDL_RenderClear(rendu);
    }	
		

    





    /************désallocation !************/
    free(cases);
    free(cases_s);
    free(S);
    SDL_DestroyRenderer(rendu);
	SDL_DestroyWindow(wdw);
	SDL_Quit();
	return EXIT_SUCCESS;

}