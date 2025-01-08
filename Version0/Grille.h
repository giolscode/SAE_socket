#ifndef GRILLE_H
#define GRILLE_H

#include <stdio.h>
#include <stdlib.h>

typedef struct {
    char symbole; 
} Case;


typedef struct {
    int longueur; 
    int largeur; 
    Case **cases; 
} Grille;


Grille* creerGrille(int longueur, int largeur);

void afficherGrille(Grille *grille);

void libererGrille(Grille *grille);

#endif 
