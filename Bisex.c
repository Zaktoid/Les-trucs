#include <stdlib.h>
#include <stdio.h>
#include <math.h>

float bissext(float (*f)(float),int iter,float a,float b)
{
    if(f(a)>0 || f(b)<0)
    {
        printf("%f,%f MAUVAIS FORMAT",f(a),f(b));
        return -1;
    }
    float m_1=a;
    float m_2=b;
    int J=0;
    while (J!=iter)
    {
        float t=(m_1+m_2)/2;
        if(f(t)<0)
            m_1=t;
        else
            m_2=t;
        J++;
    }
    return m_1;
    
}
float f(float x)
{
    return x-exp(-x);
}
int main()
{
    float a=bissext(f,19,0,1);
    int DIGS=__DECIMAL_DIG__;
    printf("%.*e\n",DIGS,a);
}