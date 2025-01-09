#include <stdio.h>
#include <stdlib.h> /* pour exit */
#include <unistd.h> /* pour read, write, close, sleep */
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h> /* pour memset */
#include <netinet/in.h> /* pour struct sockaddr_in */
#include <arpa/inet.h> /* pour htons et inet_aton */
#include <stdbool.h>
#include "Grille.h"

#define PORT 6000 
#define LG_MESSAGE 256

// Vérifie si un joueur a gagné
bool verifierVictoire(Grille *grille, char joueur) {
    int i, j;

    // Vérification des lignes
    for (i = 0; i < grille->longueur; i++) {
        bool victoire = true;
        for (j = 0; j < grille->largeur; j++) {
            if (grille->cases[i][j].symbole != joueur) {
                victoire = false;
                break;
            }
        }
        if (victoire) return true;
    }

    // Vérification des colonnes
    for (j = 0; j < grille->largeur; j++) {
        bool victoire = true;
        for (i = 0; i < grille->longueur; i++) {
            if (grille->cases[i][j].symbole != joueur) {
                victoire = false;
                break;
            }
        }
        if (victoire) return true;
    }

    // Vérification de la diagonale principale
    bool victoireDiag1 = true;
    for (i = 0; i < grille->longueur; i++) {
        if (grille->cases[i][i].symbole != joueur) {
            victoireDiag1 = false;
            break;
        }
    }
    if (victoireDiag1) return true;

    // Vérification de la diagonale secondaire
    bool victoireDiag2 = true;
    for (i = 0; i < grille->longueur; i++) {
        if (grille->cases[i][grille->largeur - i - 1].symbole != joueur) {
            victoireDiag2 = false;
            break;
        }
    }
    if (victoireDiag2) return true;

    return false;
}

// Vérifie si la grille est pleine
bool grillePleine(Grille *grille) {
    for (int i = 0; i < grille->longueur; i++) {
        for (int j = 0; j < grille->largeur; j++) {
            if (grille->cases[i][j].symbole == ' ') return false;
        }
    }
    return true;
}

int main(int argc, char const *argv[])
{
    int socketEcoute;
    Grille *morpion;
    int x, y, numCase;
    int longueur = 3;
    int largeur = 3;

    struct sockaddr_in pointDeRencontreLocal;
    socklen_t longueurAdresse;

    int socketJoueurX, socketJoueurO;
    struct sockaddr_in pointDeRencontreDistant;
    char messageRecu[LG_MESSAGE];
    int lus;

    // Création du socket
    socketEcoute = socket(AF_INET, SOCK_STREAM, 0); 
    if (socketEcoute < 0){
        perror("socket");
        exit(-1);
    }
    printf("Socket créée avec succès ! (%d)\n", socketEcoute);

    // Remplissage de sockaddrDistant (structure sockaddr_in identifiant le point d'écoute local)
    longueurAdresse = sizeof(pointDeRencontreLocal);
    memset(&pointDeRencontreLocal, 0x00, longueurAdresse); pointDeRencontreLocal.sin_family = PF_INET;
    pointDeRencontreLocal.sin_addr.s_addr = htonl(INADDR_ANY); // attaché à toutes les interfaces locales disponibles
    pointDeRencontreLocal.sin_port = htons(PORT); // port 6000

    // On demande l’attachement local de la socket
    if ((bind(socketEcoute, (struct sockaddr *)&pointDeRencontreLocal, longueurAdresse)) < 0) {
        perror("bind");
        exit(-2); 
    }
    printf("Socket attachée avec succès !\n");

    // Mise en écoute
    if (listen(socketEcoute, 2) < 0) {
        perror("listen");
        exit(-3);
    }
    printf("Socket placée en écoute passive ... \n \n");

    // Initialisation de la grille
    morpion = creerGrille(longueur, largeur);
    afficherGrille(morpion);

    // attente de connexion 
    printf("Attente de la connexion du joueur X...\n");
    socketJoueurX = accept(socketEcoute, (struct sockaddr *)&pointDeRencontreDistant, &longueurAdresse);
    if (socketJoueurX < 0) {
        perror("accept joueur X");
        exit(-4);
    }
    printf("Joueur X connecté !\n");

    printf("Attente de la connexion du joueur O...\n");
    socketJoueurO = accept(socketEcoute, (struct sockaddr *)&pointDeRencontreDistant, &longueurAdresse);
    if (socketJoueurO < 0) {
        perror("accept joueur O");
        exit(-4);
    }
    printf("Joueur O connecté !\n");

    // Réinitialisation du message pour chaque tour
    bool tourJoueurX = true;
    while (1) {
        int socketActuel = tourJoueurX ? socketJoueurX : socketJoueurO;
        char symboleActuel = tourJoueurX ? 'X' : 'O';

        // Envoyer la grille actuelle au joueur actif
        envoyerGrille(socketActuel, morpion);

        memset(messageRecu, 0, LG_MESSAGE); // Réinitialisation du message

        // On réceptionne les données du client (cf. protocole)
        lus = recv(socketActuel, messageRecu, LG_MESSAGE, 0); // ici appel bloquant
        switch (lus) {
            case -1: /* une erreur */
                perror("read");
                close(socketActuel);
                exit(-5);
            case 0:  /* la socket est fermée */
                fprintf(stderr, "La socket a été fermée par le client !\n\n");
                close(socketActuel);
                return 0;
            default:  /* réception de n octets */
                messageRecu[lus] = '\0';  // Terminer le message par un caractère nul
                printf("Message reçu : '%s' (%d octets)\n\n", messageRecu, lus);
        }

        // Interprétation des coordonnées (x et y)
        if (sscanf(messageRecu, "%d %d", &x, &y) != 2) {
            printf("Erreur : coordonnées invalides reçues : %s\n", messageRecu);
            continue;
        }

        // Vérifier que les coordonnées sont dans les limites de la grille et que la case est vide
        if (x >= 0 && x < longueur && y >= 0 && y < largeur && morpion->cases[x][y].symbole == ' ') {
            morpion->cases[x][y].symbole = 'X'; // Client joue
        } else {
            printf("Case invalide ou déjà occupée : (%d, %d).\n", x, y);
            continue;
        }

        printf("Grille après le coup du client :\n");
        afficherGrille(morpion);

        // Vérifications de victoire ou de fin de jeu pour le client
        if (verifierVictoire(morpion, symboleActuel)) {
            // Envoyer la grille complète au joueur X
            char messageGrilleX[LG_MESSAGE] = {0};
            for (int i = 0; i < morpion->longueur; i++) {
                for (int j = 0; j < morpion->largeur; j++) {
                    char symbole = morpion->cases[i][j].symbole;
                    messageGrilleX[strlen(messageGrilleX)] = (symbole == ' ') ? '.' : symbole;
                }
                strcat(messageGrilleX, "\n");
            }
            send(socketJoueurX, messageGrilleX, strlen(messageGrilleX) + 1, 0);

            // Envoyer la grille complète au joueur O
            char messageGrilleO[LG_MESSAGE] = {0};
            strcpy(messageGrilleO, messageGrilleX); // Réutilisation de la même grille
            send(socketJoueurO, messageGrilleO, strlen(messageGrilleO) + 1, 0);

            // Envoyer les messages de victoire/perte
            send(socketActuel, "Vous avez gagné !", strlen("Vous avez gagné !") + 1, 0);
            send(tourJoueurX ? socketJoueurO : socketJoueurX, "Vous avez perdu.", strlen("Vous avez perdu.") + 1, 0);

            printf("Le joueur %c a gagné !\n", symboleActuel);
            break;
        }

        if (grillePleine(morpion)) {
            // Envoyer la grille complète au joueur X
            char messageGrilleX[LG_MESSAGE] = {0};
            for (int i = 0; i < morpion->longueur; i++) {
                for (int j = 0; j < morpion->largeur; j++) {
                    char symbole = morpion->cases[i][j].symbole;
                    messageGrilleX[strlen(messageGrilleX)] = (symbole == ' ') ? '.' : symbole;
                }
                strcat(messageGrilleX, "\n");
            }
            send(socketJoueurX, messageGrilleX, strlen(messageGrilleX) + 1, 0);

            // Envoyer la grille complète au joueur O
            char messageGrilleO[LG_MESSAGE] = {0};
            strcpy(messageGrilleO, messageGrilleX); // Réutilisation de la même grille
            send(socketJoueurO, messageGrilleO, strlen(messageGrilleO) + 1, 0);

            // Envoyer les messages d'égalité
            send(socketJoueurX, "Égalité !", strlen("Égalité !") + 1, 0);
            send(socketJoueurO, "Égalité !", strlen("Égalité !") + 1, 0);

            printf("La partie est terminée par égalité.\n");
            break;
        }

        // Passer au joueur suivant
        tourJoueurX = !tourJoueurX;

    close(socketEcoute);
    return 0;
    }
}