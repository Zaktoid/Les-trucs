#include <stdlib.h>
#include <stdio.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <math.h>
#include "Mathutils.h"

complex_num App(complex_num a,complex_num c)//Application du type z²+c
{
    complex_num b;
    b.Rz=fabs(a.Rz);
    b.Iz=fabs(a.Iz);
    return sum(prod(b,b),c);
}
float Test_Diverg(int tres,complex_num c,complex_num (*application)(complex_num,complex_num),double toler)
{
complex_num iteration={0,0};
for (double i=0;i<tres;i++)
{
    iteration=application(iteration,c);//"application" est une fonction de C->C passée en argument
    if(module(iteration)>toler)//on teste par rapport au carré
    {
        return i;
    }
	if(module(sum(iteration,opp(application(iteration,c))))<=0.001)
		break;
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
	int NeedToDraw=1;
	float z=1;//facteur de zoom
	float vert=0;
	float horiz=0;
	int coefR=1;
	int coefG=1;
	int coefB=1;
	int hauteur=800;
	int largeur=800;
	int res=2;
	int IsRecording=0;
	SDL_Window *wdw;
	SDL_Renderer *rdrd;
	SDL_Surface* img;
	int f;
	char* Name=malloc(sizeof(char)*20);
    /**********intialisation d'élements SDL************/
	if(SDL_Init(SDL_INIT_VIDEO))
		SDL_Exitwitherror("Echec init");
	wdw = SDL_CreateWindow("Julia Explorer !",SDL_WINDOWPOS_CENTERED,0,hauteur,largeur,0);			
	if(!wdw)
		SDL_Exitwitherror("Creation fenêtre échouée");
	rdrd = SDL_CreateRenderer(wdw,-1,SDL_RENDERER_ACCELERATED);
	if(!rdrd)
		SDL_Exitwitherror("Echec Rendu");
	img= SDL_CreateRGBSurface(0, hauteur, largeur, 32, 0, 0, 0, 0);
	if(!img)
		SDL_Exitwitherror("Echec Surface");
	SDL_Texture* texture = SDL_CreateTextureFromSurface(rdrd, img);
	SDL_bool Launched= SDL_TRUE;


    /************Boucle principale***********************/
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
				NeedToDraw=1;//On calcule les valeurs seulement si elles ont été modifiées
				switch (event.key.keysym.sym)
				{
				
				case SDLK_RIGHT:
					horiz+=10;
					break;
				case SDLK_SPACE:
					z=1;
					break;
				case SDLK_w:
					IsRecording=1-IsRecording;
					break;
				case SDLK_LEFT:
					horiz-=10;
					break;
				case SDLK_UP:
					vert-=10;
					break;
				case SDLK_DOWN:
					vert+=10;
					break;
				case SDLK_r:
					coefR+=1;
					break;
				case SDLK_g:
					coefG+=1;
					break;
				case SDLK_b:
					coefB+=1;
					break;
				case SDLK_v:
					coefB-=1;
					break;
				case SDLK_e:
					coefR-=1;
					break;
				case SDLK_f:
					coefG-=1;
					break;
				case SDLK_a:
					z*=1.1;
					break;
				case SDLK_q:
					{z/=1.1;}
					break;
				case SDLK_c:
					SDL_SetRenderDrawColor(rdrd,0,0,0,255);
					SDL_RenderClear(rdrd);
					res+=1;
					break;
				case SDLK_x:
					SDL_SetRenderDrawColor(rdrd,0,0,0,255);
					SDL_RenderClear(rdrd);
					if(res-1)
						res-=1;
				default:
					break;
				
				}
			default:
				break;
		}}



		if(NeedToDraw)
		{
			NeedToDraw=0;
			for(double k=0;k<hauteur;k+=res)
			{
				for(double l=0;l<largeur;l+=res)
				{
					complex_num a={(k-(hauteur/2)+horiz)/(z*(hauteur/4)),
					(l-(hauteur/2)+vert)/(z*(hauteur/4))};
					float s=Test_Diverg(20*z,a,App,10);
					SDL_SetRenderDrawColor(rdrd,(coefR)*s,(coefB)*s,(coefG)*s,255);
					SDL_RenderDrawPoint(rdrd,k,l);

				}
			}
		    SDL_RenderPresent(rdrd);
			
		}
	}
	
/*************************************/
	SDL_DestroyRenderer(rdrd);
	SDL_DestroyWindow(wdw);
	SDL_FreeSurface(img);
	SDL_DestroyTexture(texture);
	SDL_Quit();
	return EXIT_SUCCESS;
}

