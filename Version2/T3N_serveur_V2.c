#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <netinet/in.h>
#include <stdbool.h>
#include "Grille.h"

#define PORT 6000
#define LG_MESSAGE 256

// Fonction pour envoyer un message à un client
void envoyerMessage(int client, const char *message) {
    if (send(client, message, strlen(message) + 1, 0) <= 0) {
        perror("Erreur d'envoi");
        exit(EXIT_FAILURE);
    }
}

// Fonction pour recevoir un message d'un client
int recevoirMessage(int client, char *buffer, size_t taille) {
    memset(buffer, 0, taille); // Nettoyage du tampon
    int recu = recv(client, buffer, taille, 0);
    if (recu < 0) {
        perror("Erreur de réception");
        exit(EXIT_FAILURE);
    } else if (recu == 0) {
        printf("Client déconnecté.\n");
        exit(EXIT_SUCCESS);
    }
    return recu;
}

// Vérifie si un joueur a gagné
bool verifierVictoire(Grille *grille, char joueur) {
    for (int i = 0; i < grille->longueur; i++) {
        // Vérification des lignes
        bool victoireLigne = true;
        for (int j = 0; j < grille->largeur; j++) {
            if (grille->cases[i][j].symbole != joueur) {
                victoireLigne = false;
                break;
            }
        }
        if (victoireLigne) return true;

        // Vérification des colonnes
        bool victoireColonne = true;
        for (int j = 0; j < grille->largeur; j++) {
            if (grille->cases[j][i].symbole != joueur) {
                victoireColonne = false;
                break;
            }
        }
        if (victoireColonne) return true;
    }

    // Vérification des diagonales
    bool victoireDiag1 = true, victoireDiag2 = true;
    for (int i = 0; i < grille->longueur; i++) {
        if (grille->cases[i][i].symbole != joueur) victoireDiag1 = false;
        if (grille->cases[i][grille->largeur - i - 1].symbole != joueur) victoireDiag2 = false;
    }
    return victoireDiag1 || victoireDiag2;
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

int main() {
    int socketEcoute, clientX, clientO;
    struct sockaddr_in sockaddrDistant;
    socklen_t longueurAdresse;

    Grille *morpion = creerGrille(3, 3);
    int x, y;
    char messageRecu[LG_MESSAGE];
    char messageEnvoye[LG_MESSAGE];
    char symboleEnJeu = 'X';
    int clientEnJeu;

    // Création du socket
    socketEcoute = socket(AF_INET, SOCK_STREAM, 0);
    if (socketEcoute < 0) {
        perror("Erreur de création du socket");
        exit(EXIT_FAILURE);
    }

    // Configuration du socket
    longueurAdresse = sizeof(sockaddrDistant);
    memset(&sockaddrDistant, 0, longueurAdresse);
    sockaddrDistant.sin_family = AF_INET;
    sockaddrDistant.sin_addr.s_addr = htonl(INADDR_ANY);
    sockaddrDistant.sin_port = htons(PORT);

    if (bind(socketEcoute, (struct sockaddr *)&sockaddrDistant, longueurAdresse) < 0) {
        perror("Erreur de liaison (bind)");
        exit(EXIT_FAILURE);
    }

    if (listen(socketEcoute, 2) < 0) {
        perror("Erreur d'écoute (listen)");
        exit(EXIT_FAILURE);
    }

    printf("En attente des connexions...\n");

    // Connexion des joueurs
    clientX = accept(socketEcoute, (struct sockaddr *)&sockaddrDistant, &longueurAdresse);
    if (clientX < 0) {
        perror("Erreur d'acceptation (clientX)");
        exit(EXIT_FAILURE);
    }
    printf("Client X connecté.\n");

    clientO = accept(socketEcoute, (struct sockaddr *)&sockaddrDistant, &longueurAdresse);
    if (clientO < 0) {
        perror("Erreur d'acceptation (clientO)");
        exit(EXIT_FAILURE);
    }
    printf("Client O connecté.\n");

    clientEnJeu = clientX;

    while (1) {
        afficherGrille(morpion);

        // Envoi des informations de tour
        envoyerMessage(clientEnJeu, "your_turn");
        envoyerMessage((clientEnJeu == clientX) ? clientO : clientX, "wait");

        // Réception des coordonnées
        recevoirMessage(clientEnJeu, messageRecu, LG_MESSAGE);

        // Extraction et validation des coordonnées
        if (sscanf(messageRecu, "%d %d", &x, &y) != 2 || x < 1 || x > 3 || y < 1 || y > 3 || morpion->cases[x - 1][y - 1].symbole != ' ') {
            envoyerMessage(clientEnJeu, "invalid");
            continue;
        }
        x--; y--; // Ajustement des indices

        // Application du coup
        morpion->cases[x][y].symbole = symboleEnJeu;

        // Vérification des conditions de victoire ou de fin de partie
        if (verifierVictoire(morpion, symboleEnJeu)) {
            sprintf(messageEnvoye, "%cwins", symboleEnJeu);
            envoyerMessage(clientX, messageEnvoye);
            envoyerMessage(clientO, messageEnvoye);
            printf("Victoire du joueur %c !\n", symboleEnJeu);
            break;
        } else if (grillePleine(morpion)) {
            envoyerMessage(clientX, "Oend");
            envoyerMessage(clientO, "Xend");
            printf("Match nul !\n");
            break;
        }

        // Changer de joueur
        clientEnJeu = (clientEnJeu == clientX) ? clientO : clientX;
        symboleEnJeu = (symboleEnJeu == 'X') ? 'O' : 'X';
    }

    // Libération des ressources
    close(clientX);
    close(clientO);
    close(socketEcoute);
    libererGrille(morpion);

    return 0;
}
