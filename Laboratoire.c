#include <stdlib.h>
#include <stdio.h>
#include <SDL2/SDL.h>
#include <math.h>
#include "Mathutils.h"
float det(complex_num C_1,complex_num C_2)
{
    return C_1.Rz*C_2.Iz-C_1.Iz*C_2.Rz;
}
int IsInTriangle(complex_num T,complex_num* Triangle)
{
    complex_num V_0=Triangle[0];
    complex_num V_1=sum(Triangle[1],opp(Triangle[0]));
    complex_num V_2=sum(Triangle[2],opp(Triangle[0]));
    float a=(det(T,V_2)-det(V_0,V_2))/det(V_1,V_2);
    float b=-(det(T,V_1)-det(V_0,V_1))/det(V_1,V_2);
    if(a<0 || b<0)return 0;
    if(a+b>=1) return 0;
    return 1;
}
void ColorTriangle(complex_num* Triangle,int* ColorVector,SDL_Renderer* rendu)
{
    complex_num a={1.0/3.0,0};
    complex_num Center=prod(sum(sum(Triangle[0],Triangle[1]),Triangle[2]),a);
    float radius=module(sum(Center,opp(Triangle[0])));
    complex_num r={radius,0};
    int X_min=(int)(sum(Center,opp(r))).Rz;
    int X_max=(int)(sum(Center,r)).Rz;
    r.Rz=0;r.Iz=radius;
    int Y_min=(int)(sum(Center,opp(r))).Iz;
    int Y_max=(int)(sum(Center,r)).Iz;
    SDL_SetRenderDrawColor(rendu,ColorVector[0],ColorVector[1],ColorVector[2],255);
    SDL_RenderDrawPoint(rendu,Center.Rz,Center.Iz);
    for(int k=X_min;k<=X_max;k++)
        {
            for(int l=Y_min;l<=Y_max;l++)
            {
                complex_num T={k,l};
                if(IsInTriangle(T,Triangle))
                {
                    SDL_RenderDrawPoint(rendu,k,l);
                }
            }
        }

}
struct Tuile
{
    int val;
    complex_num* Points;
    complex_num Center;
    int radius;
    struct Tuile** Ns;

};
typedef struct Tuile Tuile;
Tuile* Init_Tuile(complex_num Center,float r)
{
    Tuile* out=malloc(sizeof(Tuile));
    if(!out){printf("Error when initializing"); return NULL;}
    out->Center=Center;
    out->radius=r;
    out->val=0;
    complex_num r_0={r,0};
    complex_num* Points=malloc(6*sizeof(complex_num));
    if(!Points){printf("Error when initializing"); return NULL;}
    out->Ns=malloc(sizeof(Tuile*)*6);
    out->Points=Points;
    complex_num alpha={cos(6.28/6),sin(6.28/6)};
    for(int k=0;k<6;k++)
    {
    out->Ns[k]=NULL;
    r_0=prod(r_0,alpha);   
    out->Points[k]=sum(Center,prod(r_0,alpha));}
    return out;
}
void Color_Tuile(Tuile T,int* ColorVector,SDL_Renderer* rendu)
{
    for(int k=0;k<6;k++)
    {
        complex_num* Triangle=malloc(3*sizeof(complex_num));
        Triangle[0]=T.Center;
        Triangle[1]=T.Points[k];
        Triangle[2]=T.Points[(k+1)%6];
        ColorTriangle(Triangle,ColorVector,rendu);
    }
}
void CreatePavage(Tuile* T,int step,SDL_Renderer* rendu,int* V)
{
    if(step!=0)
    {
        Color_Tuile(*T,V,rendu);
        float s=sqrt(2*T->radius*T->radius*(1-cos(5*3.14/6)));
        complex_num r_0={0,s};
        complex_num alpha={cos(6.28/6),sin(6.28/6)};
        for(int k=0;k<6;k++)
        {
            if((!T->Ns[k]))
            {
            T->Ns[k]=Init_Tuile(sum(T->Center,r_0),T->radius);
            T->Ns[k]->Ns[0]=T;
            CreatePavage(T->Ns[k],step-1,rendu,V);
            r_0=prod(r_0,alpha);
            }
        }
    }

}
void FreePavage(Tuile* T)
{
    for(int k=0;k<6;k++)
    {
        free(T->Points);
        for(int k=0;k<6;k++)
        {
            if(T->Ns[k]) FreePavage(T->Ns[k]);

        }
        free(T);
    }
}
void SDL_Exitwitherror(const char *message)
{
	SDL_Log("Erreur:%s >%s \n",message,SDL_GetError());
	exit(EXIT_FAILURE);
}

int main()
{
    /**********intialisation d'élements SDL************/
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
    complex_num C={200,200};
    int r=10;
    Tuile* T=Init_Tuile(C,r);
    T->val=1;
    int V[3];V[0]=255;V[1]=10;V[2]=0;
    CreatePavage(T,10,rendu,V);
	SDL_bool Launched= SDL_TRUE;
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
                break;
            case SDL_MOUSEBUTTONDOWN:
                	//int x=event.button.x;
                	//int y=event.button.y;
			default:
				break;
		}}
        SDL_RenderPresent(rendu);

}
/*************************************/
	SDL_DestroyRenderer(rendu);
	SDL_DestroyWindow(window);
	SDL_Quit();
	return EXIT_SUCCESS;

}