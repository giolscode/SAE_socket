#include <stdio.h>
#include <stdlib.h>

typedef struct {
    char symbole; // ' ' pour vide, 'X' ou 'O' pour les joueurs
} Case;

typedef struct {
    int longueur;
    int largeur;
    Case **cases; 
} Grille;

// Fonction pour créer une grille
Grille* creerGrille(int longueur, int largeur) {
    Grille *grille = (Grille*)malloc(sizeof(Grille));
    grille->longueur = longueur;
    grille->largeur = largeur;

    grille->cases = (Case**)malloc(longueur * sizeof(Case*));
    for (int i = 0; i < longueur; i++) {
        grille->cases[i] = (Case*)malloc(largeur * sizeof(Case));
        for (int j = 0; j < largeur; j++) {
            grille->cases[i][j].symbole = ' '; // Initialement vide
        }
    }

    return grille;
}

// Fonction pour afficher la grille
void afficherGrille(Grille *grille) {
    for (int i = 0; i < grille->longueur; i++) {
        for (int j = 0; j < grille->largeur; j++) {
            printf(" %c ", grille->cases[i][j].symbole);
            if (j < grille->largeur - 1) printf("|"); // Séparateur de colonnes
        }
        printf("\n");
        if (i < grille->longueur - 1) {
            for (int j = 0; j < grille->largeur; j++) {
                printf("---");
                if (j < grille->largeur - 1) printf("+");
            }
            printf("\n");
        }
    }
}

void libererGrille(Grille *grille) {
    for (int i = 0; i < grille->longueur; i++) {
        free(grille->cases[i]); // Libère chaque ligne
    }
    free(grille->cases);       // Libère le tableau de pointeurs
    free(grille);              // Libère la structure elle-même
}


// Exemple d'utilisation
int main() {
    Grille *grille = creerGrille(3, 3);

    printf("Grille initiale :\n");
    afficherGrille(grille);

    return 0;
}