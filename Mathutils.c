#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <SDL2/SDL.h>
typedef struct {double Rq;double Iq;double Jq; double Kq;}quat;
typedef struct {double Rz;double Iz;}complex_num;
typedef complex_num(*function)(complex_num);
typedef struct 
{
    complex_num* coef;
    int deg;
}Poly;
typedef struct 
{
    quat* coef;
    int deg;
}Poly_q;
typedef struct 
{
    float inf;
    float sup;
    float actu;
    float length;
    int X;
    int Y;
    int targeted;
}slider;
typedef  complex_num(*f)(complex_num);


void Upgrage_slider(slider* slider,float incr)
{
    if((slider->actu)+incr>=(slider->inf) && (slider->actu+incr)<=slider->sup)
        slider->actu+=incr;
}
void Draw_slider(slider slider,SDL_Renderer * rend)
{
    SDL_Rect line={slider.X,slider.Y,slider.length,3};
    SDL_Rect dot={slider.X+((slider.actu-slider.inf)/slider.sup)*(slider.length),slider.Y,6,10};
    SDL_SetRenderDrawColor(rend,0,0,100,255);
    SDL_RenderDrawRect(rend,&line);
    SDL_SetRenderDrawColor(rend,0,255,0,255);
    SDL_RenderDrawRect(rend,&dot);

}

complex_num prod(complex_num a,complex_num b)//definition du produit
{
    complex_num pr;
    pr.Rz=(a.Rz)*(b.Rz)-(a.Iz)*(b.Iz);
    pr.Iz=(b.Iz)*(a.Rz)+(b.Rz)*(a.Iz);
    return pr;
}
complex_num sum(complex_num a, complex_num b)//definition de la somme
{
    complex_num sum={a.Rz+b.Rz,b.Iz+a.Iz};
    return sum;
}
complex_num Pow(complex_num z,int n)
{
    complex_num out={1,0};
    for(int k=0;k<n;k++)
    {
        out=prod(out,z);
    }
    return out;
}
complex_num Inverse(complex_num z)
{
    complex_num out = {(z.Rz)/(z.Rz*z.Rz+z.Iz*z.Iz),-1*((z.Iz)/(z.Rz*z.Rz+z.Iz*z.Iz))};
    return out;
}
complex_num opp(complex_num z)
{
    complex_num out;
    out.Rz=-z.Rz;
    out.Iz=-z.Iz;
    return out;
}
double module(complex_num a)//definition du module
{    
    return sqrt((a.Rz)*(a.Rz)+(a.Iz)*(a.Iz));
}
complex_num Exp_c(complex_num z)
{
    complex_num out={exp(z.Rz)*cos(z.Iz),exp(z.Rz)*sin(z.Iz)};
    return out;
}
complex_num mult(float A,complex_num B)
{
    B.Rz*=A;
    B.Iz*=A;
    return B;
}
complex_num Init(float A)
{
    complex_num out={A,0};
    return out;
} 
Poly* Init_Poly(int deg)
{
    Poly* out=malloc(sizeof(Poly));
    if(!out)
    {
        printf("Error init poly");
        return NULL;
    }
    out->deg=deg;
    out->coef=malloc(sizeof(complex_num)*(deg+1));
    if(!out->coef)
    {
        free(out);
        printf("Error init poly");
        return NULL;
    }
    for(int k=0;k<=deg;k++)
    {
        out->coef[k].Iz=0;
        out->coef[k].Rz=0;
    }
    return out;
}

complex_num Poly_ev(Poly P,complex_num z)
{
    complex_num out={0,0};
    for(int k=0;k<=P.deg;k++)
    {
        out=sum(out,prod(P.coef[k],Pow(z,k)));
    }
    return out;
}
double Dist(complex_num a,complex_num b)
{
    return module(sum(a,opp(b)));
}
complex_num D(complex_num(*f)(complex_num),complex_num z,complex_num h)
{
    complex_num out=prod(sum(f(sum(z,h)),opp(f(z))),Inverse(h));
    return out;

}
complex_num Newton(complex_num x0,complex_num(*f)(complex_num),int iter,complex_num h)
{
    complex_num out=x0;
    for(int k=0;k<iter;k++)
    {
        complex_num rap=prod(f(out),Inverse(prod(sum(f(sum(out,h)),opp(f(out))),Inverse(h))));
        out=sum(out,opp(rap));
    }
    return out;
}

int min(double* C,int taille)
{
    int candi=0;
    for(int k=0;k<taille;k++)
    {
        if(C[k]<C[candi])
        {
            candi=k;
        }
    }
    return candi;
}
/************************************************************************/
quat sum_q(quat a,quat b)
{
    quat out;
    out.Iq=a.Iq+b.Iq;
    out.Jq=a.Jq+b.Jq;
    out.Rq=a.Rq+b.Rq;
    out.Kq=a.Kq+b.Kq;
    return out;
}
quat prod_q(quat a ,quat b)
{
    quat out;
    out.Rq=(a.Rq)*(b.Rq)-(a.Iq)*(b.Iq)-(a.Jq)*(b.Jq)-(a.Kq)*(b.Kq);
    out.Iq=(a.Rq)*(b.Iq)+(b.Rq)*(a.Iq)+(a.Jq)*(b.Kq)-(a.Kq)*(b.Jq);
    out.Jq=(a.Rq)*(b.Jq)-(a.Iq)*(b.Kq)+(a.Jq)*(b.Rq)+(a.Kq)*(b.Iq);
    out.Kq=(a.Rq)*(b.Kq)+(a.Iq)*(b.Jq)-(a.Jq)*(b.Iq)+(a.Kq)*(b.Rq);
    return out;
}
quat Pow_q(quat z,int n)
{
    quat out={1,0,0,0};
    for(int k=0;k<n;k++)
    {
        out=prod_q(out,z);
    }
    return out;
}
Poly_q* Init_Poly_q(int deg)
{
    Poly_q* out=malloc(sizeof(Poly_q));
    if(!out)
    {
        printf("Error init poly");
        return NULL;
    }
    out->deg=deg;
    out->coef=malloc(sizeof(quat)*(deg+1));
    if(!out->coef)
    {
        free(out);
        printf("Error init poly");
        return NULL;
    }
    for(int k=0;k<=deg;k++)
    {
        out->coef[k].Iq=0;
        out->coef[k].Rq=0;
        out->coef[k].Jq=0;
        out->coef[k].Kq=0;
    }
    return out;
}
quat Poly_evq(Poly_q P,quat z)
{
    quat out={0,0,0,0};
    for(int k=0;k<=P.deg;k++)
    {
        out=sum_q(out,prod_q(P.coef[k],Pow_q(z,k)));
    }
    return out;
}
double module_q(quat a)//definition du module
{    
    return sqrt((a.Rq)*(a.Rq)+(a.Iq)*(a.Iq)+(a.Kq)*(a.Kq)+(a.Jq)*(a.Jq));
}
quat Exp_q(quat z,int iter)
{
    quat out={1,0,0,0};
    quat div={1,0,0,0};
    for(int k=1;k<iter;k++)
    {   div.Rq/=k;
        out=sum_q(out,prod_q(div,Pow_q(z,k)));
    }
    return out;
}


