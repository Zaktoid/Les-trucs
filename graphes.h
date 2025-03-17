#if defined(GRAPHES_H)
#else
#define GRAPHES_H

#define MAX 10000

struct eltadj {
	int dest;
	int info;            
	struct eltadj *suivant;
};

struct sommet {
	int label;
	int info;    
	struct sommet *suivant;      
	struct eltadj *adj;  
};

struct graphe {
	int nbS;          
	int nbA;         
	int maxS;        
	struct sommet *premierSommet;
	struct sommet *dernierSommet; 
};

typedef struct graphe GRAPHE; 
typedef struct sommet SOMMET;
typedef struct eltadj ELTADJ;

void initialiserGraphe(GRAPHE *);

int ajouterSommet(GRAPHE *, int info);

int ajouterArc(GRAPHE *, int a, int b, int info);

int supprimerSommet(GRAPHE *, int label);

int supprimerArc(GRAPHE *, int a, int b);

void supprimerGraphe(GRAPHE *);

void afficherGraphe(GRAPHE *);

int lireFichier(char *nomf, GRAPHE *);

#endif
