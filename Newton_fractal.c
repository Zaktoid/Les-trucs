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
int main()
{

    int NeedTodraw=1;
    float coefR=10;
    float coefG=10;
    float coefV=10;
    float vert=0;
    float horiz=0;
    float zoom=1;
    int hauteur=800;
    int deg=3;
    int* pdeg=&deg;
    int tr=0;
    int x;
    int y;
    complex_num C[deg];
    float a=0;
    for (int k=0;k<deg;k++)
    {
        C[k].Rz=cos(2*3.14*k/deg);
        C[k].Iz=sin(2*3.14*k/deg);
    }
    complex_num PolyR(complex_num z)
    {
        complex_num out={0,1};
        for(int k=0;k<*pdeg;k++)
        {
            out=prod(out,sum(z,opp(C[k])));
        }
        return out;
    }
	SDL_Renderer *rendu;
    SDL_Window *wdw;
	if(SDL_Init(SDL_INIT_VIDEO))
		SDL_Exitwitherror("failed init");
	wdw = SDL_CreateWindow("Newton Fractal",SDL_WINDOWPOS_CENTERED,0,hauteur,hauteur,0);
    rendu = SDL_CreateRenderer(wdw,-1,SDL_RENDERER_ACCELERATED);			
	if(!wdw)
		SDL_Exitwitherror("window creation failed");
	if(!rendu)
		SDL_Exitwitherror("renderer failed");
	SDL_bool Launched= SDL_TRUE;
        int n=6;
    slider* S=malloc(sizeof(slider)*n);
    for(int k=0;k<n;k++)
    {
        S[k].actu=1;
        S[k].inf=0;
        S[k].sup=100;
        S[k].length=100;
        S[k].X=0;
        S[k].Y=k*hauteur/n;
        S[k].targeted=0;
    }
    while (Launched)
    {
        coefR=S[0].actu;
        coefG=S[1].actu;
        coefV=S[2].actu;
        zoom=S[3].actu;
        tr=S[4].actu;
        C[0].Rz=S[5].actu;
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            switch (event.type)
            {
            case SDL_QUIT:
                Launched= SDL_FALSE;
                break;
            case SDL_MOUSEBUTTONDOWN:
                x=event.button.x;
                y=event.button.y;
                for(int k=0;k<n;k++)
                {
                    if((x>=S[k].X && x<=S[k].X+S[k].length)&& (y<=S[k].Y+5 && y>=S[k].Y-5))
                    {
                        S[k].targeted=1;
                    }
                }
                break;
            case SDL_MOUSEMOTION:
                NeedTodraw=1;
                for(int k=0;k<n;k++)
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
            case SDL_MOUSEBUTTONUP:
                NeedTodraw=0;
                for(int k=0;k<n;k++)
                    {
                        S[k].targeted=0;
                    }
                break;
            case SDL_KEYDOWN:
                NeedTodraw=1;
                switch(event.key.keysym.sym)
                { 
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
                case SDLK_h:
                    a+=0.1;
                    break;
                case SDLK_j:
                    a-=0.1;
                    break;
                case SDLK_ESCAPE:
                    Launched=SDL_FALSE;
                    break;
                default:
                    break;}
            default:
                break;
            
            }
            
        }
        SDL_SetRenderDrawColor(rendu,0,0,0,255);
        SDL_RenderClear(rendu);
        complex_num cis={cos(a),sin(a)};
        if(NeedTodraw)
        {for(int i=0;i<hauteur;i+=2)
            {
                for(int j=0;j<hauteur;j+=2)
                {
                complex_num z={zoom*((i-hauteur/2)+horiz)/hauteur,zoom*((j-hauteur/2)+vert)/hauteur};
                z=prod(z,cis);
                complex_num h={0.0001,0.0001};
                complex_num u=Newton(z,PolyR,tr,h);
                SDL_SetRenderDrawColor(rendu,55*tanh(u.Rz*coefG),105*tanh(u.Iz*coefV),255*tanh(module(u)*coefR),255);
                SDL_RenderDrawPoint(rendu,i,j);

                }
            }
        }
    for(int k=0;k<n;k++)
            Draw_slider(S[k],rendu);
    SDL_RenderPresent(rendu);
    }
    free(S);
    SDL_DestroyRenderer(rendu);
	SDL_DestroyWindow(wdw);
	SDL_Quit();
	return EXIT_SUCCESS;
}