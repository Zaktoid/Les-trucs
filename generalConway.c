#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <SDL.h>
#include "Mathutils.h"
static float PI=3.141592653;
typedef struct Polygon
{
    int xCenter;
    int Ycenter;
    int sides;
    complex_num* Vertices;
}Polygon;
void SDL_Exitwitherror(const char *message)
{
	SDL_Log("Erreur:%s >%s \n",message,SDL_GetError());
	exit(EXIT_FAILURE);
}

Polygon* Init_Polygon(int sides, float radius, int xCenter, int yCenter)
{
   Polygon* out=malloc(sizeof(Polygon));
   if(!out)
   {
    printf("Error init. polygon");
    return NULL;
   }
   out->xCenter=xCenter;
   out->Ycenter=yCenter;
   out->Vertices=malloc(sizeof(complex_num)*sides);
   out->sides=sides;
   if(!out->Vertices)
   {
    printf("Error init. polygon");
    return NULL;
   }
   complex_num rotate={cos(2*PI/sides),sin(2*PI/sides)};
   out->Vertices[0].Iz=0;
   out->Vertices[0].Rz=radius;
   for(int i=1;i<sides;i++)
     out->Vertices[i] =prod(out->Vertices[i-1],rotate);
   complex_num shift={xCenter,yCenter};
   for(int i=0;i<sides;i++)
     out->Vertices[i]=sum(out->Vertices[i],shift);

   return out;
}
void Rotate(complex_num* Cloud, complex_num Center, int size,float angle)
{
    complex_num z={0,angle};
    complex_num rotate=Exp_c(z,100);
    for (int k=0;k<size;k++)
    {
        Cloud[k]=sum(prod(sum(opp(Center),Cloud[k]),rotate),Center);
    }
}
Polygon* Init_PolygonGivenSide(int x1,int y1,int x2,int y2,int sides)
{
    float L=sqrt((x1-x2)*(x1-x2) + (y1-y2)*(y1-y2));
    float r=L/2*sin(PI/sides);
    return Init_Polygon(sides,r,x1,y1);

}
void Translate(Polygon* target,complex_num shift,int size)
{
    for(int k=0;k<size;k++)
    {
        target->Vertices[k].Iz+=shift.Iz;
        target->Vertices[k].Rz+=shift.Rz;
    }
    target->xCenter+=shift.Rz;
    target->Ycenter+=shift.Iz;
}


void DisplayCloud(complex_num* array,int size,SDL_Renderer* rendu)
{
    for(int k=0;k<size;k++)
    {
        SDL_RenderDrawLine(rendu,array[k%(size)].Rz,array[k%(size)].Iz,array[(k+1)%size].Rz,array[(k+1)%size].Iz);
    }
}
void DrawInsideOfPoly(Polygon* Poly,float[3] color,SDL_renderer* rendu)
{
    
}
complex_num GetCloudCenter(complex_num* cloud,int size)
{
    complex_num out={0,0};
    for(int k=0;k<size;k++)
    {
        complex_num z=cloud[k];
        z.Rz/=size;
        z.Iz/=size;
        out=sum(out,z);
    }
    return out;
}

int main()
{
    int hauteur=600;
    int largeur=800;
	SDL_Window *wdw;
	SDL_Renderer *rdrd;

    /**********intialisation d'élements SDL************/
	if(SDL_Init(SDL_INIT_VIDEO))
		SDL_Exitwitherror("Echec init");
	wdw = SDL_CreateWindow("Julia Explorer !",SDL_WINDOWPOS_CENTERED,0,hauteur,largeur,0);			
	if(!wdw)
		SDL_Exitwitherror("Creation fenêtre échouée");
	rdrd = SDL_CreateRenderer(wdw,-1,SDL_RENDERER_SOFTWARE);
	if(!rdrd)
		SDL_Exitwitherror("Echec Rendu");
	SDL_bool Launched= SDL_TRUE;
    Polygon* Poly=Init_Polygon(7,100,hauteur/2,largeur/2);
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
                case SDLK_e:
                    complex_num C={10,10};
                    Translate(Poly,C,Poly->sides);
                    break;
                case SDLK_r:
                    complex_num R={Poly->xCenter,Poly->Ycenter};
                    Rotate(Poly->Vertices,R,Poly->sides,PI/10);
                default:
                break;}
			default:
				break;
		}}
    SDL_SetRenderDrawColor(rdrd,255,0,40,255);
    DisplayCloud(Poly->Vertices,Poly->sides,rdrd);
    SDL_RenderPresent(rdrd);
    SDL_SetRenderDrawColor(rdrd,0,0,0,255);
    SDL_RenderClear(rdrd);

        
    }
/*************************************/
	SDL_DestroyRenderer(rdrd);
	SDL_DestroyWindow(wdw);
	SDL_Quit();
	return EXIT_SUCCESS;

}