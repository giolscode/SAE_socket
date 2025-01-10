#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
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

int main(int argc, char const *argv[]) {

    int socketEcoute, clientX, clientO;
    struct sockaddr_in sockaddrDistant;
    socklen_t longueurAdresse;

    Grille *morpion;
    int x, y;
    int longueur = 3;  // Longueur du quadrillage
    int largeur = 3;   // Largeur du quadrillage
    char messageRecuX[LG_MESSAGE], messageRecuO[LG_MESSAGE];
    char messageEnvoyeX[LG_MESSAGE], messageEnvoyeO[LG_MESSAGE];
    char symboleEnJeu;
    int clientEnJeu = 1; // 1 pour X, 2 pour O

    // Création du socket
    socketEcoute = socket(AF_INET, SOCK_STREAM, 0);
    if (socketEcoute < 0) {
        perror("socket");
        exit(-1);
    }
    printf("Socket créée avec succès ! (%d)\n", socketEcoute);

    // Remplissage de sockaddrDistant
    longueurAdresse = sizeof(sockaddrDistant);
    memset(&sockaddrDistant, 0x00, longueurAdresse);
    sockaddrDistant.sin_family = PF_INET;
    sockaddrDistant.sin_addr.s_addr = htonl(INADDR_ANY);
    sockaddrDistant.sin_port = htons(PORT);

    // Attachement local de la socket
    if (bind(socketEcoute, (struct sockaddr *)&sockaddrDistant, longueurAdresse) < 0) {
        perror("bind");
        exit(-2);
    }
    printf("Socket attachée avec succès !\n");

    // Mise en écoute
    if (listen(socketEcoute, 5) < 0) {
        perror("listen");
        exit(-3);
    }
    printf("Socket en écoute...\n");

    // Initialisation de la grille
    morpion = creerGrille(longueur, largeur);
    afficherGrille(morpion);

    // Attente de connexion des deux joueurs
    printf("Attente de la connexion des joueurs...\n");

    clientX = accept(socketEcoute, (struct sockaddr *)&sockaddrDistant, &longueurAdresse);
    if (clientX < 0) {
        perror("accept clientX");
        exit(-4);
    }
    printf("Client X connecté.\n");

    clientO = accept(socketEcoute, (struct sockaddr *)&sockaddrDistant, &longueurAdresse);
    if (clientO < 0) {
        perror("accept clientO");
        exit(-5);
    }
    printf("Client O connecté.\n");

    // Boucle principale du jeu
    while (1) {
        int socketEnJeu = (clientEnJeu == 1) ? clientX : clientO;
        symboleEnJeu = (clientEnJeu == 1) ? 'X' : 'O';

        // Envoi du message au joueur actuel
        if (clientEnJeu == 1) {
            sprintf(messageEnvoyeX, "C'est votre tour (%c). Entrez un numéro de case : ", symboleEnJeu);
            if (send(clientX, messageEnvoyeX, strlen(messageEnvoyeX) + 1, 0) <= 0) {
                perror("Erreur lors de l'envoi à client X");
                break;
            }
        } else {
            sprintf(messageEnvoyeO, "C'est votre tour (%c). Entrez un numéro de case : ", symboleEnJeu);
            if (send(clientO, messageEnvoyeO, strlen(messageEnvoyeO) + 1, 0) <= 0) {
                perror("Erreur lors de l'envoi à client O");
                break;
            }
        }

        // Envoi des infos de jeu
        sprintf(messageEnvoyeX, "L'adversaire a joué sur la case %d.", x * largeur + y);
        sprintf(messageEnvoyeO, "L'adversaire a joué sur la case %d.", x * largeur + y);
        if (send(clientX, messageEnvoyeX, strlen(messageEnvoyeX) + 1, 0) <= 0) {
            perror("Erreur lors de l'envoi des infos à client X");
            break;
        }
        if (send(clientO, messageEnvoyeO, strlen(messageEnvoyeO) + 1, 0) <= 0) {
            perror("Erreur lors de l'envoi des infos à client O");
            break;
        }

        // Réception des coordonnées
        if (clientEnJeu == 1) {
            memset(messageRecuX, 0, LG_MESSAGE);
            int lus = recv(clientX, messageRecuX, LG_MESSAGE, 0);
            // if (lus <= 0) {
            //     printf("Client X déconnecté.\n");
            //     break;
            // }
            int caseNum = messageRecuX[0] - '0';
            x = caseNum / largeur;
            y = caseNum % largeur;
        } 
        else {
            memset(messageRecuO, 0, LG_MESSAGE);
            int lus = recv(clientO, messageRecuO, LG_MESSAGE, 0);
            // if (lus <= 0) {
            //     printf("Client O déconnecté.\n");
            //     break;
            // }
            int caseNum = messageRecuO[0] - '0';
            x = caseNum / largeur;
            y = caseNum % largeur;
        }

        // Vérification et application du coup
        if (x < 0 || x >= longueur || y < 0 || y >= largeur || morpion->cases[x][y].symbole != ' ') {
            if (clientEnJeu == 1) {
                send(clientX, "Coup invalide.", strlen("Coup invalide.") + 1, 0);
            } else {
                send(clientO, "Coup invalide.", strlen("Coup invalide.") + 1, 0);
            }
            continue;
        }
        morpion->cases[x][y].symbole = symboleEnJeu;

        // Vérification de la victoire ou de la fin du jeu
        if (verifierVictoire(morpion, symboleEnJeu)) {
            sprintf(messageEnvoyeX, "%cwins", symboleEnJeu);
            sprintf(messageEnvoyeO, "%cwins", symboleEnJeu);
            send(clientX, messageEnvoyeX, strlen(messageEnvoyeX) + 1, 0);
            send(clientO, messageEnvoyeO, strlen(messageEnvoyeO) + 1, 0);
            break;
        }

        if (grillePleine(morpion)) {
            sprintf(messageEnvoyeX, "Match nul.");
            sprintf(messageEnvoyeO, "Match nul.");
            send(clientX, messageEnvoyeX, strlen(messageEnvoyeX) + 1, 0);
            send(clientO, messageEnvoyeO, strlen(messageEnvoyeO) + 1, 0);
            break;
        }

        // Synchronisation et mise à jour de l'état
        sprintf(messageEnvoyeX, "L'adversaire a joué sur la case %d.", x * largeur + y);
        sprintf(messageEnvoyeO, "L'adversaire a joué sur la case %d.", x * largeur + y);
        send(clientX, messageEnvoyeX, strlen(messageEnvoyeX) + 1, 0);
        send(clientO, messageEnvoyeO, strlen(messageEnvoyeO) + 1, 0);

        // Passer au joueur suivant
        clientEnJeu = (clientEnJeu == 1) ? 2 : 1;
    }

    // Nettoyage
    close(clientX);
    close(clientO);
    close(socketEcoute);
    libererGrille(morpion);
    return 0;
}