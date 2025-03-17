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
typedef struct 
{
    float* Coordinates;
    float* vals;
    float MaxVAl;
    int size;
}Kernel;

Kernel* InitKer(complex_num* coord,int size,float* vals)
{
    Kernel* out=malloc(sizeof(Kernel));
    out->Coordinates=malloc(sizeof(float)*size*2);
    out->vals=malloc(sizeof(float)*size);
    out->size=size;
    float M=0;
    for(int k=0;k<size;k++)
    {
        out->Coordinates[k]=coord[k].Rz;
        out->Coordinates[k+size]=coord[k].Iz;
        out->vals[k]=vals[k];
        M+=vals[k];
    }
    out->MaxVAl=M;
    return out;
}
void DisplayKer(Kernel* Ker,SDL_Renderer* rendu,int size,float fact)
{
    for(int k=0;k<Ker->size;k++)
    {
        float s=Ker->vals[k];
        SDL_SetRenderDrawColor(rendu,s*fact,s*fact,s*fact,255);
        SDL_RenderDrawPoint(rendu,size/2 +Ker->Coordinates[k],size/2+Ker->Coordinates[k+Ker->size]);
    }
}

void MoveKer(Kernel* Ker,complex_num A)
{
    for(int k=0;k<Ker->size;k++)
    {
        Ker->Coordinates[k]+=A.Rz;
        Ker->Coordinates[k+Ker->size]+=A.Iz;
    }
}

float Convolute(Kernel* Ker,float* Grid,int size,int x,int y)
{
    float out=0;
    for(int k=0;k<(Ker->size);k++)
    {
        int l=x+Ker->Coordinates[k];
        int m=y+Ker->Coordinates[k+Ker->size];
        l=(l>=0&&l<size)*l+0*(l>=size)+size*(l<0);
        m=(m>=0&&m<size)*m+0*(m>=size)+size*(m<0);
        out+=Ker->vals[k]*Grid[l*size+m];
    }
    return (out/Ker->MaxVAl);
}

float Growth(float u,float mu,float sigma)
{
    return 2*exp(-pow((u-mu)/sigma,2)/2.0)-1;
}
float Bell(float u,float mu,float sigma)
{
    return exp(-pow((u-mu)/sigma,2)/2.0);
}

int main()
{
	srand(time(NULL));
    float mu=0.15;
    float sigma=0.015;
    int p=1;
    float z=1;
    float horiz=0;
    float vert=0;
    /**********intialisation d'élements SDL************/
    float T=20;
	int hauteur=300;
    int hauteur1=100;
    int hauteur2=200;
	SDL_Window *window;
    SDL_Window *window1;
    SDL_Window *window2;
	SDL_Renderer *rendu;
    SDL_Renderer *rendu1;
    SDL_Renderer *rendu2;
    float R_2=5;
    float R_1=1;
    complex_num* S=malloc(sizeof(complex_num)*4*R_2*R_2);
    float* v=malloc(sizeof(float)*4*R_2*R_2);
    int J=0;
    int res=2;
    complex_num C={0,0};
    for(int k=-R_2;k<=R_2;k++)
    {
        for(int l=-R_2;l<=R_2;l++)
        {
            complex_num T={k,l};
            float R=Dist(T,C);
            float r=R/R_2;
            float s=Bell(r,0.5,0.15);
            float r_1=R/R_1;
            if((r<1 && r>0.5) )
            {
                S[J]=T;
                J++;
                v[J]=s;
            }
        }
    }
    Kernel* K=InitKer(S,J,v);
    printf("%d \n",J);
    free(S);
	if(SDL_Init(SDL_INIT_VIDEO))
		SDL_Exitwitherror("failed init");
	window= SDL_CreateWindow("Nom fenêtre",SDL_WINDOWPOS_CENTERED,0,hauteur*res,hauteur*res,0);			
	window1= SDL_CreateWindow("Nom fenêtre",SDL_WINDOWPOS_CENTERED,0,hauteur1,hauteur1,0);
    window2= SDL_CreateWindow("Nom fenêtre",SDL_WINDOWPOS_CENTERED,0,hauteur2,hauteur2,0);				
    if(!window)
		SDL_Exitwitherror("window creation failed");
	rendu = SDL_CreateRenderer(window,-1,SDL_RENDERER_ACCELERATED);
    rendu1 = SDL_CreateRenderer(window1,-1,SDL_RENDERER_ACCELERATED);
    rendu2 = SDL_CreateRenderer(window2,-1,SDL_RENDERER_ACCELERATED);
	if(!rendu)
		SDL_Exitwitherror("renderer failed");
	SDL_bool Launched= SDL_TRUE;
    float* Array=malloc(sizeof(float)*hauteur*hauteur);
    float* Array_n=malloc(sizeof(float)*hauteur*hauteur);
    for(int k=0;k<hauteur*hauteur;k++)
    {
        Array[k]=(float)(rand()%100)/100;
        Array_n[k]=0;
    }
    C.Iz+=hauteur/2;
    C.Rz+=hauteur/2;
    C.Rz=hauteur/2;
    C.Iz=hauteur/2;
    int Cr=255;
    int Cb=255;
    int Cv=255;
    SDL_Rect A;
    A.w=res;
    A.h=res;
    while (Launched)
	{
		SDL_Event event;  
        DisplayKer(K,rendu1,100,Cb);
        SDL_SetRenderDrawColor(rendu2,255,255,255,255);
        for(int k=0;k<hauteur2;k++)
        {
            float x=((float)k+horiz)/(z*hauteur2);
            float y=z*(Growth(x,mu,sigma)+vert);
            SDL_RenderDrawPoint(rendu2,k,(hauteur2-y));
        }      
		while(SDL_PollEvent(&event))
		{
			switch (event.type)
			{
			case SDL_KEYDOWN:
				switch (event.key.keysym.sym)
				{
                case SDLK_q:
                    sigma-=0.001;
                    printf("s%f,m:%f \n",sigma,mu);
                    break;
                case SDLK_s:
                    sigma+=0.001;
                    printf("s%f,m:%f \n",sigma,mu);
                    break;
                case SDLK_m:
                    mu+=0.001;
                    printf("s%f,m:%f \n",sigma,mu);
                    break;
                case SDLK_l:
                    mu-=0.001;
                    printf("s%f,m:%f \n",sigma,mu);
                    break;
                case SDLK_w:
                    for(int k=0;k<hauteur*hauteur;k++)
                        Array[k]=0;
                    break;
                case SDLK_p:
                    p=1-p;
                    break;
                case SDLK_b:
                    Cb*=10;
                    break;
                case SDLK_g:
                    Cv+=10;
                    break;
                case SDLK_r:
                    Cr+=10;
                    break;
                case SDLK_x:
                    Launched=SDL_FALSE;
                    break;
                case SDLK_UP:
                    vert+=0.1/z;
                    break;
                case SDLK_DOWN:
                    vert-=0.1/z;
                    break;
                case SDLK_LEFT:
                    horiz-=0.1/z;
                    break;
                case SDLK_RIGHT:
                    horiz+=0.1/z;
                    break;
                case SDLK_z:
                    z*=1.1;
                    break;
                case SDLK_a:
                    z/=1.1;
                    break;
				default:
					break;
				}
                break;
            	case SDL_MOUSEBUTTONDOWN:
                	int x=event.button.x;
                	int y=event.button.y;
                    for(int k=-1;k<=1;k++)
                    {
                        for(int l=-1;l<=1;l++)
                        {
                            if((x+k)<hauteur&&(y+l)<hauteur&&0<=(x+k)&&0<=(y+l))
                            Array[(k+x)*hauteur+y+l]=(float)(rand()%100)/99;
                        }
                    }
					break;
			default:
				break;
		}}
        if(!p)
        {for(int k=0;k<hauteur;k++)
        {
            for(int l=0;l<hauteur;l++)
            {
                float u=Convolute(K,Array,hauteur,k,l);
                float h=2*exp(-(u-mu)*(u-mu)/(2*sigma*sigma))-1;
                Array_n[k*hauteur+l]=Array[k*hauteur+l]+(1.0/T)*(h);
                if(Array_n[k*hauteur+l]<0) Array_n[k*hauteur+l]=0;
                if(Array_n[k*hauteur+l]>1) Array_n[k*hauteur+l]=1;

            }
        }}
        for(int k=0;k<hauteur;k++)
        {
            for(int l=0;l<hauteur;l++)
            {
                    if(!p)Array[k*hauteur+l]=Array_n[k*hauteur+l];
                    float s= Array[k*hauteur+l];
                    A.x=k*res;
                    A.y=l*res;
                    SDL_SetRenderDrawColor(rendu,s*Cr,s*Cb,s*Cv,255);
                    SDL_RenderFillRect(rendu,&A);
                }
            }
        SDL_RenderPresent(rendu);
        SDL_RenderPresent(rendu1);
        SDL_RenderPresent(rendu2);
        SDL_RenderClear(rendu1);
        SDL_SetRenderDrawColor(rendu2,0,0,0,255);
        SDL_RenderClear(rendu2);
		
}
/*************************************/
	SDL_DestroyRenderer(rendu);
    SDL_DestroyRenderer(rendu1);
    SDL_DestroyRenderer(rendu2);
	SDL_DestroyWindow(window);
    SDL_DestroyWindow(window1);
    SDL_DestroyWindow(window2);
    free(Array);
    free(Array_n);
    free(K->vals);
    free(K->Coordinates);
    free(K);
	SDL_Quit();
	return EXIT_SUCCESS;

}