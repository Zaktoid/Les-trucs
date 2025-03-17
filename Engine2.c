#include <stdlib.h>
#include <stdio.h>
#include <SDL2/SDL.h>
#include <math.h>
#define EPSILON 1e-9
void SDL_Exitwitherror(const char *message)
{
	SDL_Log("Erreur:%s >%s \n",message,SDL_GetError());
	exit(EXIT_FAILURE);
}

void solve_3x3_system(float A[3][3], float b[3], float x[3]) {
    int i, j, k;
    float max, temp;
    int pivot_row;

    // Augmented matrix [A|b]
    float augmented[3][4];
    for (i = 0; i < 3; i++) {
        for (j = 0; j < 3; j++) {
            augmented[i][j] = A[i][j];
        }
        augmented[i][3] = b[i];
    }

    // Gaussian elimination with partial pivoting
    for (i = 0; i < 3; i++) {
        // Find the pivot row
        max = fabs(augmented[i][i]);
        pivot_row = i;
        for (j = i + 1; j < 3; j++) {
            if (fabs(augmented[j][i]) > max) {
                max = fabs(augmented[j][i]);
                pivot_row = j;
            }
        }

        // Check for singular matrix
        if (fabs(max) < EPSILON) {
			x[0]=x[1]=x[2]=INFINITY;
            return;
        }

        // Swap rows if needed
        if (pivot_row != i) {
            for (j = 0; j < 4; j++) {
                temp = augmented[i][j];
                augmented[i][j] = augmented[pivot_row][j];
                augmented[pivot_row][j] = temp;
            }
        }

        // Eliminate entries below the pivot
        for (j = i + 1; j < 3; j++) {
            float factor = augmented[j][i] / augmented[i][i];
            for (k = i; k < 4; k++) {
                augmented[j][k] -= factor * augmented[i][k];
            }
        }
    }

    // Back substitution
    for (i = 2; i >= 0; i--) {
        x[i] = augmented[i][3];
        for (j = i + 1; j < 3; j++) {
            x[i] -= augmented[i][j] * x[j];
        }
        x[i] /= augmented[i][i];
    }


}
////////////////////////////////////////////////////////////////

typedef struct
{
	float* V_1;
	float* V_2;
	float* P;
	float* b_1;
	float* b_2;
}lilguy;

lilguy* InitGuy(float* V_1,float* V_2,float* P,float* b_1,float* b_2)
{
	lilguy* out=malloc(sizeof(lilguy));
	out->V_1=V_1;
	out->V_2=V_2;
	out->P=P;
	out->b_1=b_1;
	out->b_2=b_2;
	return out;

}
void DestroyLilguy(lilguy* in)
{
	printf("feur");
	free(in->V_1);
	free(in->V_2);
	free(in->P);
	free(in->b_1);
	free(in->b_2);
	free(in);
}

void TestCollide(lilguy* O,float d[3],float Out[4],float Pos[3])
{
	float A[3][3];
	float NP[3];
	for(int i=0;i<3;i++)
	{
		A[i][0]=d[i];
		A[i][1]=O->V_1[i];
		A[i][2]=O->V_2[i];
		NP[i]=O->P[i]-Pos[i];
	}
	float Params[3];
	solve_3x3_system(A,NP,Params);
	if(Params[0]==INFINITY)
		Out[0]=0;
	else if(Params[1]>O->b_1[1] || Params[1]<O->b_1[0])
		Out[0]=0;
	else if(Params[2]>O->b_2[1] || Params[2]<O->b_2[0])
		Out[0]=0;
	else
	{
		Out[0]=1;
		Out[1]=Params[0];
		Out[2]=Params[1];
		Out[3]=Params[2];
	}

}

void RayCast(lilguy** Objects,float K,int L,SDL_Renderer* U,int hauteur,float* Pos,int Nb,float* theta)
{
	float Out[4];
	int Collided=0;
	float w_1[3];
	for(int i=-L/2;i<L/2;i+=1)
	{
		for(int k=-L/2;k<L/2;k+=1)
		{	
			float v_1[3]={i,k,K};
			w_1[0]=cos(theta[0])*v_1[0]+sin(theta[0])*v_1[2];
			w_1[1]=v_1[1];
			w_1[2]=cos(theta[0])*v_1[2]-sin(theta[0])*v_1[0];
			v_1[0]=w_1[0];
			v_1[1]=w_1[1]*cos(theta[1])-w_1[2]*sin(theta[1]);
			v_1[2]=w_1[1]*sin(theta[1])+w_1[2]*cos(theta[1]);
			//SearchNearestObject
			for(int l=0;l<Nb;l++)
			{	
				TestCollide(Objects[l],v_1,Out,Pos);
				int b=(Out[1]>0 && Out[0]);
				int u=(int)Out[2]%2;
				int d=(int)Out[3]%2;
				if(b) 
				{
					Collided=1;
					SDL_SetRenderDrawColor(U,(255),u*(255),d*(255),255);
					SDL_RenderDrawPoint(U,i+hauteur/2,k+hauteur/2);
				}
				if(!Collided)
					{
						SDL_SetRenderDrawColor(U,0,0,0,255);
					}
				Collided=0;
			
			}
			

		}
			
	}

}

int main(int argc , char **argv)
{
    /**********intialisation d'élements SDL************/
	int hauteur=400;
	SDL_Window *window;
	SDL_Renderer *rendu;
	SDL_Texture *text;
	SDL_Surface *surface;
	if(SDL_Init(SDL_INIT_VIDEO))
		SDL_Exitwitherror("failed init");
	window= SDL_CreateWindow("Nom fenêtre",SDL_WINDOWPOS_CENTERED,0,hauteur,hauteur,0);			
	if(!window)
		SDL_Exitwitherror("window creation failed");
	rendu = SDL_CreateRenderer(window,-1,SDL_RENDERER_SOFTWARE);
	if(!rendu)
		SDL_Exitwitherror("renderer failed");
	SDL_bool Launched= SDL_TRUE;
	float theta[2]={0,0};
	int L=hauteur;
	int K=L/2;
	float b_1[2]={-100,100};
	float b_2[2]={0,100};
	float P[3]={0,0,0};
	float a=0;
	float b=3.14/2.0;
	float V_1[3]={sin(a)*cos(b),cos(a)*cos(b),sin(b)};
	float W_1[3]={cos(a)*cos(b),-sin(a)*cos(b),sin(b)};
	float V_2[3]={cos(a),-sin(a),0};
	float W_2[3]={-sin(a),cos(a),0};
	float C[3]={0,0,0};
	lilguy* O=InitGuy(V_1,V_2,P,b_1,b_2);
	lilguy** Guys=malloc(sizeof(lilguy)*2);
	Guys[0]=O;
	float Q[3]={0,0,0};
	Guys[1]=InitGuy(W_1,W_2,Q,b_1,b_2);
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
				case SDLK_w:
					theta[1]+=0.1;
					break;
				case SDLK_x:
					theta[1]-=0.1;
					break;
				case SDLK_d:
					C[0]++;
					break;
				case SDLK_q:
					C[0]--;
					break;
				case SDLK_a:
					C[1]++;
					break;
				case SDLK_e:
					C[1]--;
					break;
				case SDLK_z:
					C[2]+=1*cos(theta[0]);
					C[0]+=1*sin(theta[0]);
					break;
				case SDLK_s:
					C[2]-=1*cos(theta[0]);
					C[0]-=1*sin(theta[0]);
					break;
				case SDLK_l:
					a+=1.57/100.0;
					float V_1[3]={sin(a)*cos(b),cos(a)*cos(b),sin(b)};
					float V_2[3]={cos(a),-sin(a),0};
					O->V_1=V_1;
					O->V_2=V_2;
					break;
				case SDLK_m:
					b+=1.57/100.0;
					float V[3]={sin(a)*cos(b),cos(a)*cos(b),sin(b)};
					float W[3]={cos(a),-sin(a),0};
					O->V_1=V;
					O->V_2=W;
					break;
				case SDLK_c:
					theta[0]+=0.1;
					break;
				case SDLK_v:
					theta[0]-=0.1;
					break;
				default:
					break;
				}
			default:
				break;
		}}
		RayCast(Guys,K,L,rendu,hauteur,C,2,theta);
		SDL_RenderPresent(rendu);
		SDL_SetRenderDrawColor(rendu,0,0,0,255);
		SDL_RenderClear(rendu);
		
}
/*************************************/
	free(O);
	SDL_DestroyRenderer(rendu);
	SDL_DestroyWindow(window);
	SDL_Quit();
	return EXIT_SUCCESS;

}