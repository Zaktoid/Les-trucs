#include <stdio.h>
#include "graphes.h"

void initialiserGraphe(GRAPHE *g)
{
	g->nbS=0;        
	g->nbA=0;
	g->maxS=0;
	g->premierSommet=NULL;
	g->dernierSommet=NULL;
}

int ajouterSommet(GRAPHE *g, int info)
{
	SOMMET *pointeur;
	g->maxS++;
	pointeur=(SOMMET *) malloc(sizeof(SOMMET));
	if (pointeur == NULL)
		{
			printf("Erreur! Memoire insuffisante pour creer un sommet\n");
			return -1; 
		}
		else
		{
			pointeur->label=g->maxS;
			pointeur->info=info;
			pointeur->suivant=NULL;
			pointeur->adj=NULL;
			if (g->nbS == 0)
				{
					g->premierSommet=pointeur;
					g->dernierSommet=pointeur;
				}
			else
				{
					g->dernierSommet->suivant=pointeur;
					g->dernierSommet=pointeur;
				}
			g->nbS++;
			return pointeur->label;
		}
}

int ajouterArc(GRAPHE *g, int a, int b, int info)
{
	SOMMET *psommet, *psommet2;
	ELTADJ *padj, *precedent;
	psommet=g->premierSommet;

	/* on parcourt les sommets jusqu'a trouver a */
	while (psommet != NULL)
		{
			if (psommet->label == a) break;
			psommet=psommet->suivant;
		}
	if (psommet == NULL)
		{
			printf("Erreur! Creation d'un arc dont l'origine n'existe pas\n");
			return -1;
		}
	else /* on a trouver a */
		{
			padj=psommet->adj;

			/* on parcourt les sommets pour trouver b */
			psommet2=g->premierSommet;
			while (psommet2 != NULL)
				{
					if (psommet2->label == b) break;
					psommet2=psommet2->suivant;
				}
			if (psommet2 == NULL)
				{
					printf("Erreur! Creation d'un arc dont l'extremite n'existe pas\n");
					return -2;
				}
			else /* on a trouver a et b */
				{
					if (padj == NULL) /* la liste d'adjacence est vide */
						{
							padj=(ELTADJ *) malloc(sizeof(ELTADJ));
							if (padj == NULL)
								{
									printf("Erreur! Memoire insuffisante pour creer un sommet\n");
									return -3; 
								}
							else
								{
									psommet->adj=padj; /* premier element de la liste d'adjacence */
									padj->suivant=NULL; 
								}
						}
					else /* la liste d'adjacence est non vide, on la parcourt pour voir si b s'y trouve */
						{
							if (padj->dest > b)
								{
									padj=(ELTADJ *) malloc(sizeof(ELTADJ));
									if (padj == NULL)
										{
											printf("Erreur! Memoire insuffisante pour creer un sommet\n");
											return -3; 
										}
									else
										{
											padj->suivant=psommet->adj;
											psommet->adj=padj;
										}
								}
							else
								{
									while (padj != NULL)
										{
											if (padj->dest == b) {padj->info=info; break;} /* l'arc existe, update info */
											if (padj->dest > b) {padj=NULL; break;} /* on depasse b sans le trouver */
											precedent=padj;
											padj=padj->suivant;
										}
									if (padj == NULL) /* l'arc n'existe pas, il faut le creer */
										{
											padj=(ELTADJ *) malloc(sizeof(ELTADJ));
											if (padj == NULL)
												{
													printf("Erreur! Memoire insuffisante pour creer un sommet\n");
													return -3; 
												}
											else
												if (precedent->suivant==NULL) /* element ajouter a la fin */
													{
														precedent->suivant=padj;
														padj->suivant=NULL; 
													}
												else /* element ajouter "au milieu" pour garder ordre */
													{
														padj->suivant=precedent->suivant;
														precedent->suivant=padj; 
													}
										}
								}
						}
					padj->dest=b;
					padj->info=info;
					g->nbA++;
				}
			return 0;
		}
}
	

int supprimerSommet(GRAPHE *g, int a)
{
	SOMMET *psommet, *precedent;
	ELTADJ *padj, *suivant, *precedent_adj;
	int flag_premier_sommet, flag_premier_arc;
	if (g->premierSommet == NULL)
		{
			printf("Erreur! Graphe vide, suppression impossible\n");
			return -1;
		}
	else
		{
			psommet=g->premierSommet;
			flag_premier_sommet=1;
			while (psommet != NULL)
				{
					if (psommet->label == a) break;
					else
						{
							flag_premier_sommet=0;
							precedent=psommet;
							psommet=psommet->suivant;
						}
				}
			if (psommet == NULL)
				{
					printf("Erreur! Le sommet a supprimer n'existe pas\n");
					return -1;
				}
			else
				{
					if (psommet->suivant == NULL) g->dernierSommet=precedent;
					
					if (flag_premier_sommet == 1) g->premierSommet=psommet->suivant;
					else precedent->suivant=psommet->suivant;
					padj=psommet->adj;
					free(psommet);
					g->nbS--;
					while (padj != NULL)
						{
							suivant=padj->suivant;
							free(padj);
							g->nbA--;
							padj=suivant;
						}
				}
			
			/* il faut aussi supprimer les arcs ayant le sommet a supprimer comme extremite */
			psommet=g->premierSommet;
			while (psommet != NULL)
				{
					padj=psommet->adj;
					flag_premier_arc=1; 
					while (padj !=NULL)
						{
							if (padj->dest == a) break;
							else
								{
									flag_premier_arc=0;
									precedent_adj=padj;
									padj=padj->suivant; 
								}
						}
					if (padj != NULL)
						{
							if (flag_premier_arc == 1) psommet->adj=padj->suivant;
							else precedent_adj->suivant=padj->suivant;
							free(padj);
							g->nbA--;
						}
					psommet=psommet->suivant;
				}
			return 0;
		}
}

int supprimerArc(GRAPHE *g, int a, int b)
{
	SOMMET *psommet;
	ELTADJ *padj, *precedent_adj;
	int flag_premier_arc;
	if (g->premierSommet == NULL)
		{
			printf("Erreur! Graphe vide, suppression impossible\n");
			return -1;
		}
	else
		{
			psommet=g->premierSommet;
			while (psommet != NULL)
				{
					if (psommet->label == a) break;
					else psommet=psommet->suivant;
				}
			if (psommet == NULL)
				{
					printf("Erreur! L'extremite de l'arc a supprimer n'existe pas\n");
					return -1;
				}
			else
				{
					padj=psommet->adj;
					flag_premier_arc=1; 
					while (padj !=NULL)
						{
							if (padj->dest == b) break;
							else
								{
									flag_premier_arc=0;
									precedent_adj=padj;
									padj=padj->suivant; 
								}
						}
					if (padj != NULL)
						{
							if (flag_premier_arc == 1) psommet->adj=padj->suivant;
							else precedent_adj->suivant=padj->suivant;
							free(padj);
							g->nbA--;
						}
					else
						{
							printf("Erreur! L'extremite de l'arc a supprimer n'existe pas\n");
							return -1;
						}
				}
			return 0;
		}
}

void supprimerGraphe(GRAPHE *g)
{
	SOMMET *psommet,*temps;
	ELTADJ *padj,*tempadj;
	psommet=g->premierSommet;
	while (psommet !=NULL)
		{
			padj=psommet->adj;
			while (padj !=NULL)
				{
					tempadj=padj;
					padj=padj->suivant;
					free(tempadj);
				}
			temps=psommet;
			psommet=psommet->suivant;
			free(temps);
		}
	initialiserGraphe(g);
}

void afficherGraphe(GRAPHE *g)
{
	SOMMET *psommet;
	ELTADJ *padj;
	if (g->premierSommet == NULL) printf("graphe vide\n");
	else
		{
			printf("nbS=%d , nbA=%d, label max.=%d\n", g->nbS, g->nbA, g->maxS);
			psommet=g->premierSommet;
			do
				{
					printf("\n");
					printf("Sommet de label: %d , info: %d\n", psommet->label, psommet->info);
					if (psommet->adj == NULL) printf(" -> ce sommet n'a aucun arc sortant\n ");
					else
						{
							padj=psommet->adj;
							do
								{
									printf(" -> arc de %d vers %d avec l'info. %d \n", psommet->label, padj->dest, padj->info);
									padj=padj->suivant;
								}
							while (padj != NULL);
						}
					printf("\n");
					psommet=psommet->suivant;
				}
			while (psommet != NULL); 
		}
}

int lireFichier(char *nomf, GRAPHE *g)
{
  FILE *fp;
  char ligne[MAX+1]; 
  int temp,i,j,nbS1,nbLigne,sommet,nbElt,creerArc;

	initialiserGraphe(g);
  fp=fopen(nomf,"r"); /* ouvre un fichier en lecture */
  nbLigne=0; /* compte les lignes du fichier */
  sommet=0; /* label du sommet en cours */
  nbS1=0; /* compte les sommets de la 1ere ligne */
  while (fgets(ligne,MAX,fp) != NULL)
    { 
      nbLigne++; /* compte le nombre de lignes du fichier */
			
      if (ligne[0] != '\n') /* on passe les lignes vides */
				{
					i=0; 
					if (nbS1 == 0) /* compte les sommets de la 1ere ligne */
						{
							nbS1=1; 
							while (ligne[i] != '\n') 
								{
									if (ligne[i] == ',') nbS1++;
									i++;
								}
							for (j=1; j<=nbS1; j++) 
								{
									ajouterSommet(g,0);
								}
							i=0; /* on relit la 1ere ligne */
						}
					
					sommet++; /* origine des arcs */
					nbElt=0; /* controle le nombre d'arcs crees */
					while (ligne[i] != '\n')
						{
							temp=0; /* va contenir l'extremite */
							creerArc=1;
							while (ligne[i] != ',' && ligne[i] != '\n')
								{
									while (ligne[i]==' ' || ligne[i]=='\t') {i++;}
									if ((ligne[i]>'9' || ligne[i]<'0') && ligne[i]!='x') 
										{
											printf("Erreur à la ligne %d !\n",nbLigne);
											supprimerGraphe(g);
											return -1; /* pas des chiffres ! */
										}
									if (ligne[i]=='x') creerArc=0;
									temp=10*temp+ligne[i]-'0';
									i++; 
									while (ligne[i]==' ' || ligne[i]=='\t') {i++;}
								}
							if (ligne[i] == ',') i++; 
							nbElt++;
							if (nbElt<=nbS1 && creerArc==1) ajouterArc(g,sommet,nbElt,temp); /* ligne pas trop longue */
						} 
					if (nbElt != nbS1) /* pas le bon nombre de champs sur ligne */
						{
							printf("Erreur à la ligne %d !\n",nbLigne);
							supprimerGraphe(g);
							return -1; /* pas le bon nombre de champs */
						}
				}
    }
  fclose(fp);
	return 0;
}
