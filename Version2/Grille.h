/******************************************************************************
 * 
 * Nom du Projet   : Tic-Tac-Toe (Saé Socket)
 * Auteurs         : DEMOL Alexis - LOSAT Giovanni - DEBRUYNE Lucas 
 * Date de Création: 07/01/25
 * Dernière Mise à Jour : 11/01/25
 * 
 *****************************************************************************/

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

void envoyerGrille(int socketActuel, Grille *grille);

#endif 
