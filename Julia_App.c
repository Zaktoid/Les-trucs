#include <stdlib.h>
#include <stdio.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <math.h>
#include "Mathutils.h"

complex_num c={-0.4, 0.6};//Variable global pour la "nature" de la fractale

complex_num App(complex_num a)//Application du type z²+c
{

    return sum(prod(a,a),c);
}

float Test_Diverg(int tres,complex_num a,complex_num (*application)(complex_num),double toler)
{
complex_num iteration=a;
for (double i=0;i<tres;i++)
{
    iteration=application(iteration);//"application" est une fonction de C->C passée en argument
    if(iteration.Rz*iteration.Rz+iteration.Iz*iteration.Iz>toler*toler)//on teste par rapport au carré
    {
        return i;
    }
	if(module(sum(iteration,opp(application(iteration))))<0)
		break;
}
return 0;
}
void SDL_Exitwitherror(const char *message)
{
	SDL_Log("Erreur:%s >%s \n",message,SDL_GetError());
	exit(EXIT_FAILURE);
}

complex_num Syrac(complex_num in)
{
	complex_num arg={0,3.1415926535};
	complex_num exp=sum(Init(1.0),opp(Exp_c(prod(arg,in))));
	complex_num T_1=mult(0.5,in);
	complex_num T_2=mult(0.25,sum(mult(2,in),Init(1)));
	T_2=prod(T_2,exp);
	return sum(T_1,T_2);
}



int main()
{
	int NeedToDraw=1;
	float z=2;//facteur de zoom
	int coefR=10;
	int coefG=12;
	int coefB=12;
	int res=5;
	int hauteur=800;
	int largeur=800;
	float horiz=50;
	float vert=-10;
	int IsRecording=0;
	SDL_Window *wdw;
	SDL_Renderer *rdrd;
	float tres=89.099213;
    /**********intialisation d'élements SDL************/
	if(SDL_Init(SDL_INIT_VIDEO))
		SDL_Exitwitherror("Echec init");
	wdw = SDL_CreateWindow("Julia Explorer !",SDL_WINDOWPOS_CENTERED,0,hauteur,largeur,0);			
	if(!wdw)
		SDL_Exitwitherror("Creation fenêtre échouée");
	rdrd = SDL_CreateRenderer(wdw,-1,SDL_RENDERER_ACCELERATED);
	if(!rdrd)
		SDL_Exitwitherror("Echec Rendu");
	SDL_SetRenderDrawBlendMode(rdrd,SDL_BLENDMODE_BLEND);
	SDL_bool Launched= SDL_TRUE;
	SDL_Rect R;
	float w=0;
    /************Boucle principale***********************/
	while (Launched)
	{
		R.h=R.w=res;
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
				case SDLK_s:
					w+=0.001/z;
					break;
				case SDLK_d:
					w-=0.001/z;
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
				case SDLK_k:
					c.Rz+=(0.001)/z;
					break;
				case SDLK_l:
					c.Iz+=(0.001)/z;
					break;
				case SDLK_i:
					c.Rz-=(0.001)/z;
					break;
				case SDLK_o:
					c.Rz-=(0.001)/z;
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
					break;
				case SDLK_t:
					tres+=0.1;
					break;
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
					float s=Test_Diverg(1000,a,Syrac,tres);
					R.x=k;
					R.y=l;
					if((a.Rz>=-w && a.Rz<=w) || (a.Iz>=-w && a.Iz<=w))
						SDL_SetRenderDrawColor(rdrd,255,255,255,255);
					else
						SDL_SetRenderDrawColor(rdrd,(coefR)*s,(coefB)*s,(coefG)*s,255);
					SDL_RenderFillRect(rdrd,&R);

				}
			}
		    SDL_RenderPresent(rdrd);
			
		}
	}
	
/*************************************/
	printf("tres:%f,h:%f,v:%f,Cr:%d,Cb:%d,Cg:%d \n",tres,horiz,vert,coefR,coefB,coefG);
	SDL_DestroyRenderer(rdrd);
	SDL_DestroyWindow(wdw);
	SDL_Quit();
	return EXIT_SUCCESS;
}

