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
float Integ1D(float (*f)(float), float a, float b, int N) {
    // Ensure N is even for Simpson's Rule
    if (N % 2 != 0) {
        N++;
    }

    float h = (b - a) / N; // Step size
    float integral = 0.0f;

    // Simpson's Rule implementation
    integral += f(a) + f(b); // First and last terms

    for (int i = 1; i < N; i++) {
        float x = a + i * h;
        if (i % 2 == 0) {
            integral += 2 * f(x); // Even index
        } else {
            integral += 4 * f(x); // Odd index
        }
    }

    integral *= h / 3.0f; // Final scaling factor
    return integral;
}
float Convolute(float (*f)(float), float (*g)(float), float in, int N) {
    float integrand(float t) {
        return f(t) * g(in - t);
    }

    float a = -10.0f;
    float b = 10.0f; 


    return Integ1D(integrand, a, b, N);
}


int main(int argc , char **argv)
{
	srand(time(NULL));
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
	SDL_bool Launched= SDL_TRUE;
	SDL_SetRenderDrawColor(rendu,255,0,0,255);
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