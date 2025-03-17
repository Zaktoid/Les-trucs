#include <stdlib.h>
#include <stdio.h>
#include <SDL2/SDL.h>
#include <math.h>
#include "Mathutils.h"

float suite(float mu,float x0,unsigned iter)
{  
    float out=x0;
    for (unsigned k=0;k<iter;k++)
    {
        out=mu*(1-out);
    }

    return out;
}
float Lyopanov(float (*S)(float ,float ,float ,int ),float x_0,unsigned iter,float A,float B,float C)
{
    float s=x_0;
    float out=0;
    for(unsigned k=2;k<iter;k++)
    {   
        s=s*S(A,B,C,k)*(1-s);
        out+=log(fabs(S(A,B,C,k)*(1-2*s)))/iter;
        if(4<=out)
            {
                out=4;
                break;
            }
        if(out<=-4)
            {
                out=-4;
                break;
            }

    }
    return out;
}
float S(float A,float B,float C,int iter)
{
    if(iter%3==0)
        return A;
    if(iter%3==1)
        return B;
    return C;
}
void SDL_Exitwitherror(const char *message)
{
	SDL_Log("Erreur:%s >%s \n",message,SDL_GetError());
	exit(EXIT_FAILURE);

}   
int main()
{
    int hauteur=600;
    float z=1;
    float horiz=0;
    float ver=0;
    int N=200;
    float CR=29.4;
    float CG=22.3;
    float CB=-12.8;
    float C=1;
    float x_0=0.5;
    float theta=0;
    int res=5;
    SDL_Window *wdw;
	SDL_Renderer *rendu;
	if(SDL_Init(SDL_INIT_VIDEO))
		SDL_Exitwitherror("failed init");
	wdw = SDL_CreateWindow("CC mec",SDL_WINDOWPOS_CENTERED,0,hauteur,hauteur,0);			
	if(!wdw)
		SDL_Exitwitherror("window creation failed");
	rendu = SDL_CreateRenderer(wdw,-1,SDL_RENDERER_SOFTWARE);
	if(!rendu)
		SDL_Exitwitherror("renderer failed");
	SDL_bool Launched= SDL_TRUE;
    SDL_SetRenderDrawColor(rendu,255,0,0,255);
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
            case SDL_KEYDOWN:
                switch (event.key.keysym.sym)
                {
                case SDLK_e:
                    C*=1.03;
                    break;
                case SDLK_r:
                    C/=1.03;
                    break;
                case SDLK_a:
                    z+=0.1;
                    break;
                case SDLK_z:
                    z-=0.1;
                    break;
                case SDLK_LEFT:
					horiz-=10;
					break;
                case SDLK_RIGHT:
                    horiz+=10;
                    break;
				case SDLK_UP:
					ver-=10;
					break;
				case SDLK_DOWN:
					ver+=10;
					break;
                case SDLK_q:
                    CR+=0.1;
                    break;
                case SDLK_w:
                    CR-=0.1;
                    break;
                case SDLK_s:
                    CG+=0.1;
                    break;
                case SDLK_x:
                    CG-=0.1;
                    break;
                case SDLK_d:
                    CB+=0.1;
                    break;
                case SDLK_c:
                    CB-=0.1;
                    break;
                case SDLK_m:
                    x_0+=0.01;
                    break;
                case SDLK_l:
                    x_0-=0.01;
                    break;
                case SDLK_v:
                    theta-=0.1;
                    break;
                case SDLK_b:
                    theta+=0.1;
                    break;
                case SDLK_j:
                    res+=1;
                    break;
                case SDLK_k:
                    if(res!=1)
                    res-=1;
                    break;
                default:
                    break;
                }
            }
        
        }
    for(float k=0;k<hauteur;k+=res)
    {
        for(float l=0;l<hauteur;l+=res)
        {
            float A=(k+horiz)/(z*(hauteur/4));
            float B=(l+ver)/(z*(hauteur/4));
            complex_num u={A,B};
            complex_num cis={cos(theta),sin(theta)};
            u=prod(u,cis);
            float lambda=Lyopanov(S,x_0,N,u.Rz,u.Iz,C);
            SDL_SetRenderDrawColor(rendu,tanh(lambda/4+1)*40*CR,tanh(lambda/4+1)*40*CG,tanh(lambda/4+1)*40*CB,0255);
            SDL_RenderDrawPoint(rendu,k,l);
        }
    }
    SDL_RenderPresent(rendu);
    SDL_SetRenderDrawColor(rendu,0,0,0,255);
    SDL_RenderClear(rendu);
    
    }
    printf("R:%f,G:%f;B:%f, z:%f,h:%f,v:%f \n",CR,CG,CB,z,horiz,ver);
    SDL_DestroyRenderer(rendu);
	SDL_DestroyWindow(wdw);
	SDL_Quit();
	return EXIT_SUCCESS;
}