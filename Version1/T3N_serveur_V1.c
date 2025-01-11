/******************************************************************************
 * 
 * Nom du Projet   : Tic-Tac-Toe (Saé Socket)
 * Auteurs         : DEMOL Alexis - LOSAT Giovanni - DEBRUYNE Lucas 
 * Date de Création: 07/01/25
 * Dernière Mise à Jour : 11/01/25
 * 
 *****************************************************************************/

#include <stdio.h>
#include <stdlib.h> /* pour exit */
#include <unistd.h> /* pour read, write, close, sleep */
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h> /* pour memset */
#include <netinet/in.h> /* pour struct sockaddr_in */
#include <arpa/inet.h> /* pour htons et inet_aton */
#include <stdbool.h>
#include "Grille.h" /* Class Grille */

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

    int socketDialogue;
    struct sockaddr_in pointDeRencontreDistant;
    char messageRecu[LG_MESSAGE]; /* le message de la couche Application ! */
    int ecrits, lus; /* nb d’octets ecrits et lus */
    int retour;

    // Création du socket
    socketEcoute = socket(AF_INET, SOCK_STREAM, 0); 
    if (socketEcoute < 0){
        perror("socket");
        exit(-1);
    }
    printf("Socket créée avec succès ! (%d)\n", socketEcoute);

    // Remplissage de pointDe RencontreLocal (structure sockaddr_in identifiant le point d'écoute local)
    longueurAdresse = sizeof(pointDeRencontreLocal);
    memset(&pointDeRencontreLocal, 0x00, longueurAdresse); pointDeRencontreLocal.sin_family = PF_INET;
    pointDeRencontreLocal.sin_addr.s_addr = htonl(INADDR_ANY); // attaché à toutes les interfaces locales disponibles
    pointDeRencontreLocal.sin_port = htons(6000); // port 6000

    // On demande l’attachement local de la socket
    if ((bind(socketEcoute, (struct sockaddr *)&pointDeRencontreLocal, longueurAdresse)) < 0) {
        perror("bind");
        exit(-2); 
    }
    printf("Socket attachée avec succès !\n");

    // Mise en écoute
    if (listen(socketEcoute, 5) < 0) {
        perror("listen");
        exit(-3);
    }
    printf("Socket placée en écoute passive ... \n \n");

    // Initialisation de la grille
    morpion = creerGrille(longueur, largeur);
    afficherGrille(morpion);

    // attente de connexion 
    printf("Attente d’une demande de connexion...\n\n");
    socketDialogue = accept(socketEcoute, (struct sockaddr *)&pointDeRencontreDistant, &longueurAdresse);
    if (socketDialogue < 0) {
        perror("accept");
        close(socketDialogue);
        close(socketEcoute);
        exit(-4);
    }
    printf("Connexion acceptée. Le jeu commence !\n");

    // Réinitialisation du message pour chaque tour
    while (1) {
        memset(messageRecu, 0, LG_MESSAGE); // Réinitialisation du message

        // On réceptionne les données du client (cf. protocole)
        lus = recv(socketDialogue, messageRecu, LG_MESSAGE, 0); // ici appel bloquant
        switch (lus) {
            case -1: /* une erreur */
                perror("read");
                close(socketDialogue);
                exit(-5);
            case 0:  /* la socket est fermée */
                fprintf(stderr, "La socket a été fermée par le client !\n\n");
                close(socketDialogue);
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
        if (verifierVictoire(morpion, 'X')) {
            send(socketDialogue, "Xwins", strlen("Xwins") + 1, 0);
            printf("Le joueur X a gagné !\n");
            break;
        }

        if (grillePleine(morpion)) {
            send(socketDialogue, "Xend", strlen("Xend") + 1, 0);
            printf("La grille est pleine. Fin de la partie.\n");
            break;
        }

        // Tour du serveur de jouer
        int caseServeur;
        do {
            caseServeur = rand() % (longueur * largeur) + 1; // Entre 1 et 9
            x = (caseServeur - 1) / largeur; // Ligne
            y = (caseServeur - 1) % largeur; // Colonne
        } while (morpion->cases[x][y].symbole != ' ');

        // Placer le symbole du serveur
        morpion->cases[x][y].symbole = 'O';
        printf("Serveur joue à la case : %d \n", caseServeur);

        // Envoyer la case choisie au client
        char caseEnvoyee[3];
        sprintf(caseEnvoyee, "%d", caseServeur);
        if (send(socketDialogue, caseEnvoyee, strlen(caseEnvoyee) + 1, 0) <= 0) {
            perror("Erreur lors de l'envoi des données...");
            break;
        }

        // Vérifications de victoire ou de fin de jeu pour le serveur
        if (verifierVictoire(morpion, 'O')) {
            send(socketDialogue, "Owins", strlen("Owins") + 1, 0);
            printf("Le joueur O a gagné !\n");
            break;
        }

        if (grillePleine(morpion)) {
            send(socketDialogue, "Oend", strlen("Oend") + 1, 0);
            printf("La grille est pleine. Fin de la partie.\n");
            break;
        }

        // Continuer le jeu si aucune condition de fin n'est atteinte
        send(socketDialogue, "continue", strlen("continue") + 1, 0);
    }

        // Afficher la grille après le choix du serveur
        printf("Grille après le coup du serveur :\n");
        afficherGrille(morpion);

    close(socketEcoute);
    return 0;
}