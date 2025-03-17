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
////////////////////////////////////////////////////
typedef struct 
{
	float* P;
	float* V;
	float* F_R;
	float m;
	
}Particle;

Particle* Creat_Obj(float* P)
{
	Particle* out=malloc(sizeof(Particle));
	if(!out){return NULL;}
	out->P=P;
    out->V=malloc(sizeof(int)*3);
    out->F_R=malloc(sizeof(int)*3);
    out->F_R[0]=out->F_R[1]=out->F_R[2]=0;
    return out;
}

void Upgrade(Particle* in,float timestep)
{

	float v0_x=in->V[0];
	float v0_y=in->V[1];
    float v0_z=in->V[2];
	float d_vx=in->F_R[0]/in->m;
	float d_vy=in->F_R[1]/in->m;
    float d_vz=in->F_R[2]/in->m;
	in->V[0]+=d_vx;
	in->V[1]+=d_vy;
    in->V[2]+=d_vz;
	in->P[0]+=v0_x/timestep;
	in->P[1]+=v0_y/timestep;
    in->P[2]+=v0_z/timestep;

}

int Gravi(Particle* A,Particle* B, float G)
{
	float x_1=A->P[0];
	float x_2=B->P[0];
	float y_1=A->P[1];
	float y_2=B->P[1];
    float z_1=A->P[2];
	float z_2=B->P[2];
	float dist=(sqrt((x_1-x_2)* (x_1-x_2)+ (y_1-y_2)*(y_1-y_2)+(z_1-z_2)*(z_1-z_2)));
	if(dist==0)
	{
	return 1;
	}
	float G_x=-(x_1-x_2)/dist;
	float G_y=-(y_1-y_2)/dist;
    float G_z=-(y_1-y_2)/dist;
	A->F_R[0]+=(A->m)*(B->m)*G_x/(G*dist*dist);
	A->F_R[1]+=(A->m)*(B->m)*G_y/(G*dist*dist);
    A->F_R[2]+=(A->m)*(B->m)*G_z/(G*dist*dist);
	B->F_R[0]+=-(A->m)*(B->m)*G_x/(G*dist*dist);
	B->F_R[1]+=-(A->m)*(B->m)*G_y/(G*dist*dist);
    B->F_R[2]+=-(A->m)*(B->m)*G_z/(G*dist*dist);
	return 0;
}


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
float PointOnLine(float* Pos,float d[3],float T[3])
{
    float S=(T[0]-Pos[0])/d[0];
    printf("%f,%f,%f \n",S,(T[1]-Pos[1])/d[1],(T[2]-Pos[2])/d[2]);
    if(S==(T[1]-Pos[1])/d[1]  && S==(T[2]-Pos[2])/d[2])
        return S;
    else 
        return -1;

}

void CastParticle(Particle* In,int K,int L,float* pos,float* theta, int hauteur,SDL_Renderer* U)
{
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
            float S=PointOnLine(pos,v_1,In->P);
            if(S>0 && S!=INFINITY)
            {
                SDL_SetRenderDrawColor(U,(255),S*(50),S*(25),255);
				SDL_RenderDrawPoint(U,i+hauteur/2,k+hauteur/2);
            }		
        }

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
				int u=Out[2];
				int d=Out[3];
                int r=(u*u+d*d)*(u*u+d*d)+400*u*(u*u+d*d)-40000*d*d;
				if(b) 
				{
					Collided=1;
					SDL_SetRenderDrawColor(U,(205),(r<0)*(255),(r<0)*(255),255);
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
	float b_1[2]={-1000,1000};
	float b_2[2]={0,1000};
	float P[3]={0,0,0};
	float a=0;
	float b=3.14/2.0;
	float V_1[3]={sin(a)*cos(b),cos(a)*cos(b),sin(b)};
	float W_1[3]={cos(a)*cos(b),-sin(a)*cos(b),sin(b)};
	float V_2[3]={cos(a),-sin(a),0};
	float W_2[3]={-sin(a),cos(a),0};
	float C[3]={0,0,0};
	lilguy* O=InitGuy(V_1,V_2,P,b_1,b_2);
	lilguy** Guys=malloc(sizeof(lilguy*)*2);
	Guys[0]=O;
	float Q[3]={0,0,0};
	Guys[1]=InitGuy(W_1,W_2,Q,b_1,b_2);
    	int nb=20;
	Particle** Things=malloc(nb*sizeof(Particle*));
	for(int k=0;k<nb;k++)
	{
		float* P=malloc(3*sizeof(float));
		P[0]=rand()%hauteur;
		P[1]=rand()%hauteur;
        P[2]=rand()%hauteur;
		Things[k]=Creat_Obj(P);
		Things[k]->m=1;

	}
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
					C[2]+=2*cos(theta[0]);
					C[0]+=2*sin(theta[0]);
					break;
				case SDLK_s:
					C[2]-=2*cos(theta[0]);
					C[0]-=2*sin(theta[0]);
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
        for(int i=0;i<nb;i++)
			{ 
			    for(int j=(i+1)%nb;j!=i;j=(j+1)%nb)
				{

					//Gravi(Things[i],Things[j],5000);
				}
			}
		RayCast(Guys,K,L,rendu,hauteur,C,2,theta);
        for(int i=0;i<nb;i++)
            {
                //CastParticle(Things[i],K,L,C,theta,hauteur,rendu);
                //Upgrade(Things[i],3);
            }


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