#include <stdlib.h>
#include <stdio.h>
#include <SDL2/SDL.h>
#include <math.h>
#include "Mathutils.h"

quat f(quat a)
{
    quat c={0,1,0,0};
    return sum_q(prod_q(prod_q(a,Pow_q(a,2)),Pow_q(a,2)),c);
}
void SDL_Exitwitherror(const char *message)
{
	SDL_Log("Erreur:%s >%s \n",message,SDL_GetError());
	exit(EXIT_FAILURE);


}
int main()
{
    int slice1=0;
    int slice2=0;
    int NeedTodraw=1;
    float coefR=10;
    float coefG=10;
    float coefV=10;
    float vert=0;
    float horiz=0;
    float zoom=1;
    int hauteur=800;
    SDL_Window *wdw;
	SDL_Renderer *rendu;
	if(SDL_Init(SDL_INIT_VIDEO))
	SDL_Exitwitherror("failed init");
	wdw = SDL_CreateWindow("quack",SDL_WINDOWPOS_CENTERED,0,hauteur,hauteur,0);
    rendu = SDL_CreateRenderer(wdw,-1,SDL_RENDERER_SOFTWARE);	
	if(!wdw)
		SDL_Exitwitherror("window creation failed");
	if(!rendu)
		SDL_Exitwitherror("renderer failed");
	SDL_bool Launched= SDL_TRUE;
    while (Launched)
    {
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            switch (event.type)
            {
            case SDL_QUIT:
                Launched= SDL_FALSE;
                break;
            break;
                case SDL_KEYDOWN:
                NeedTodraw=1;
                switch(event.key.keysym.sym)
                {   case(SDLK_s):
                        coefR*=1.1;
                        break;
                    case(SDLK_q):
                        coefR=coefR/1.1;
                        break;
                    case(SDLK_w):
                        coefG=coefG/1.1;
                        break;
                    case(SDLK_x):
                        coefG=coefG*1.1;
                        break;
                    case(SDLK_d):
                        coefV=coefV/1.1;
                        break;
                    case(SDLK_f):
                        coefV*=1.1;
                        break;
                case SDLK_RIGHT:
                    horiz+=10;
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
                case SDLK_z:
                    zoom*=1.1;
                    break;
                case SDLK_a:
                    zoom/=1.1;
                    break;
                case SDLK_ESCAPE:
                    Launched=SDL_FALSE;
                    break;
                case SDLK_b:
                    slice1=(1-slice1);
                    break;
                case SDLK_n:
                    slice2=(1-slice2);
                    break;
                default:
                    break;}}
            
        }
            
        
        if(NeedTodraw)
        {
            for(int k=0;k<hauteur;k+=1)
            for(int r=0;r<hauteur;r+=1)
            {
                quat z=
                {((slice1+slice2)%2)*zoom*((r-hauteur/2)+horiz)/hauteur,
                zoom*((k-hauteur/2)+vert)/hauteur,
                zoom*((k-hauteur/2)+horiz)/hauteur,
                zoom*((r-hauteur/2)+horiz)/hauteur};
                quat u=f(z);
                switch (slice1+slice2)
                {
                case 2:
                    SDL_SetRenderDrawColor(rendu,tanh(u.Jq*coefR)*255,tanh(u.Kq*coefG),tanh(u.Rq*coefV)*255,255);
                    break;
                case 1:
                    SDL_SetRenderDrawColor(rendu,tanh(u.Rq*coefR)*255,tanh(u.Kq*coefG),tanh(u.Iq*coefV)*255,255);
                    break;
                case 0:
                    SDL_SetRenderDrawColor(rendu,tanh(u.Jq*coefR)*255,tanh(u.Rq*coefG),tanh(u.Iq*coefV)*255,255);
                default:
                    break;
                }
                SDL_RenderDrawPoint(rendu,r,k);

            }
        NeedTodraw=0; 
        SDL_RenderPresent(rendu);
        }
    }
    SDL_DestroyRenderer(rendu);
	SDL_DestroyWindow(wdw);
	SDL_Quit();
	return EXIT_SUCCESS;
        
    }


