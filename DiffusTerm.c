#include <stdlib.h>
#include <stdio.h>
#include <SDL.h>
#include <math.h>
#include <time.h>



typedef struct
{
    float Temp;
    float C_bois;
}Point;

void SDL_Exitwitherror(const char *message)
{
	SDL_Log("Erreur:%s >%s \n",message,SDL_GetError());
	exit(EXIT_FAILURE);
}
void InitArray(Point *Array,Point*Array_n,int taille)
{
    for (int k=0;k<taille*taille;k++)
    {
            int s=rand();
            Array[k].C_bois=Array_n[k].C_bois=4*(s%13==1);
            Array[k].Temp=Array_n[k].Temp=0;
    }
}
float Delta(Point *Array,int x,int y,int taille,float D)
{
    float s=0;
    for(int i=-1;i<=1;i++)
	{
		for(int j=-1;j<=1;j++)
		{
            if(i||j)
                s+= Array[x*taille+y].Temp-Array[(x+i)*taille+y+j].Temp;
		}
	}
    return s*D;
}

int main()
{
    srand(time(0));
    int hauteur=700;
    int taille=700;
    Point* Grille=malloc(taille*taille*sizeof(Point));
    Point* Grille_n=malloc(taille*taille*sizeof(Point));
    InitArray(Grille,Grille_n,taille);
    float T=100;
    /***********************/
	SDL_Window *wdw;
	SDL_Renderer *rendu;
	if(SDL_Init(SDL_INIT_VIDEO))
		SDL_Exitwitherror("failed init");
	wdw = SDL_CreateWindow("Nom fenêtre",SDL_WINDOWPOS_CENTERED,0,hauteur,hauteur,0);			
	if(!wdw)
		SDL_Exitwitherror("window creation failed");
	rendu = SDL_CreateRenderer(wdw,-1,SDL_RENDERER_SOFTWARE);
	if(!rendu)
		SDL_Exitwitherror("renderer failed");
    SDL_bool Launched= SDL_TRUE;
    int pause=1;
    float k_c=1;
    float N_k=1;
    float h=0.0005;
    float T_0=0;
    /************************/
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
				default:
					break;
				}
				break;
            case SDL_MOUSEBUTTONDOWN:
                int x=event.button.x;
                int y=event.button.y;
                for(int i=-100;i<=100;i++)
                {
                    {
                        for(int l=-100;l<=100;l++)
                            Grille[taille*((y+i)*taille/hauteur)+(taille*(x+l)/hauteur)].Temp+=40;
                    }
                }                
                break;

			default:
				break;
			}
		}
		if(!pause)
		{	
		    for(int l=1;l<(taille-1);l++)
		        {
			        for (int k=1;k<(taille-1);k++)
			        {
                        /******Combustion*******/
                        if(Grille[l*taille+k].Temp>=20)
                        {
                            Grille_n[l*taille+k].C_bois-=(k_c*Grille[l*taille+k].C_bois)/T;
                            Grille_n[l*taille+k].Temp+=(k_c*N_k*Grille[l*taille+k].C_bois-h*(Grille[l*taille+k].Temp-T_0))/T;
                            printf("wo,%f,%f\n",Grille_n[l*taille+k].C_bois,Grille_n[l*taille+k].Temp);
                        }
                        /*********Diffusion thermique*********/
                        //Grille_n[l*taille+k].Temp+=(k_c*N_k*Grille[l*taille+k].C_bois+Delta(Grille,k,l,taille,1)-h*(Grille[l*taille+k].Temp-T_0));
		            }
                }
        }
		for(int l=0;l<(taille);l++)
		{
			for (int k=0;k<taille;k++)
			{
				if(!pause)
                {
                    Grille[l*taille+k].C_bois=Grille_n[l*taille +k].C_bois;
                    Grille[l*taille+k].Temp=Grille_n[l*taille +k].Temp;
                }
				
				SDL_SetRenderDrawColor(rendu,Grille[l*taille+k].Temp,Grille[l*taille+k].C_bois*30,0,255);
				SDL_RenderDrawPoint(rendu,k,l);
			}
		}
		SDL_RenderPresent(rendu);
		
}
    /***********************/
    free(Grille);
    free(Grille_n);
    SDL_DestroyRenderer(rendu);
	SDL_DestroyWindow(wdw);
	SDL_Quit();
	return EXIT_SUCCESS;    
}