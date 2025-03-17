#ifndef MATHUTILS_H_
#define MATHUTILS_H_
typedef struct {double Rq;double Iq;double Jq; double Kq;}quat;
typedef struct {double Rz;double Iz;}complex_num;
typedef struct 
{
    complex_num* coef;
    int deg;
}Poly;
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
typedef struct 
{
    quat* coef;
    int deg;
}Poly_q;

void Upgrage_slider(slider* slider,float incr);
complex_num opp(complex_num z);

void Draw_slider(slider slider,SDL_Renderer * rendu);

complex_num prod(complex_num a,complex_num b);//definition du produit

complex_num sum(complex_num a, complex_num b);//definition de la somme

complex_num Pow(complex_num z,int n);

complex_num Inverse(complex_num z);


double module(complex_num a);

complex_num Exp_c(complex_num z);
complex_num mult(float A,complex_num B);
complex_num Init(float A);


Poly* Init_Poly(int deg);
double Dist(complex_num a,complex_num b);

complex_num Poly_ev(Poly P,complex_num z);
complex_num D(complex_num(*f)(complex_num),complex_num z,complex_num h);
complex_num Newton(complex_num x0,complex_num(*f)(complex_num),int iter,complex_num h);
int min(double* C,int taille);
quat prod_q(quat a ,quat b);
quat sum_q(quat a,quat b);
quat Pow_q(quat z,int n);
Poly_q* Init_Poly_q(int deg);
quat Poly_evq(Poly_q P,quat z);
double module_q(quat a);
quat Exp_q(quat z,int iter);

#endif
