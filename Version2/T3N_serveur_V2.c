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
    int socketEcoute, socketDialogue, clientX, clientO, socketEnJeu;
    struct sockaddr_in sockaddrDistant;
    socklen_t longueurAdresse;
    Grille *morpion;
    int x, y, numCase;
    int longueur = 3;  // Longueur quadrillage
    int largeur = 3;   // Largeur quadrillage
    char messageRecu[LG_MESSAGE];
    char messageEnvoyee[LG_MESSAGE];
    char symboleEnJeu;
    int lus;
    int clientEnJeu = 1;  // Joueur X commence en premier

    // Création du socket
    socketEcoute = socket(AF_INET, SOCK_STREAM, 0); 
    if (socketEcoute < 0){
        perror("socket");
        exit(-1);
    }
    printf("Socket créée avec succès ! (%d)\n", socketEcoute);

    // Remplissage de sockaddrDistant (structure sockaddr_in identifiant le point d'écoute local)
    longueurAdresse = sizeof(sockaddrDistant);
    memset(&sockaddrDistant, 0x00, longueurAdresse); 
    sockaddrDistant.sin_family = PF_INET;
    sockaddrDistant.sin_addr.s_addr = htonl(INADDR_ANY); // attaché à toutes les interfaces locales disponibles
    sockaddrDistant.sin_port = htons(PORT); // port 6000

    // On demande l’attachement local de la socket
    if ((bind(socketEcoute, (struct sockaddr *)&sockaddrDistant, longueurAdresse)) < 0) {
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

    // Attente de connexion des deux clients 
    printf("Attente de la connexion du joueur X...\n\n");

    clientX = accept(socketEcoute, (struct sockaddr *)&sockaddrDistant, &longueurAdresse);
    if (socketDialogue < 0) {
        perror("accept");
        close(socketDialogue);
        close(socketEcoute);
        exit(-4);
    }
    printf("Client X connecté, maintenant Client O... !\n");

    clientO = accept(socketEcoute, (struct sockaddr *)&sockaddrDistant, &longueurAdresse);
    if (socketDialogue < 0) {
        perror("accept");
        close(socketDialogue);
        close(socketEcoute);
        exit(-5);
    }

    // Réinitialisation du message pour chaque tour
    while (1) {
        memset(messageRecu, 0, LG_MESSAGE); // Réinitialisation du message

        // Pour savoir le tour de quel joueur
        if (clientEnJeu == 1){
            socketEnJeu = clientX;
            symboleEnJeu = 'X';
        } else {
            socketEnJeu = clientO;
            symboleEnJeu = 'O';
        }

        // Envoi du message au client concerné
        send(socketEnJeu,"C'est votre tour.", strlen("C'est votre tour.") + 1, 0);

        // On réceptionne les données du client (cf. protocole)
        lus = recv(socketDialogue, messageRecu, LG_MESSAGE, 0); // appel bloquant
        switch (lus) {
            case -1:
                perror("read");
                close(socketDialogue);
                exit(-6);
            case 0:
                fprintf(stderr, "La socket a été fermée par le client !\n\n");
                close(socketDialogue);
                return 0;
            default:
                messageRecu[lus] = '\0';  // Terminer le message par un caractère nul
                printf("Message reçu : '%s' (%d octets)\n\n", messageRecu, lus);
        }

        // Interprétation des coordonnées (x et y)
        sscanf(messageRecu, "%d %d", &x, &y);

        // Vérifier que les coordonnées sont dans les limites de la grille et que la case est vide
        if (x < 0 || x >= longueur || y < 0 || y >= largeur || morpion->cases[x][y].symbole != ' ') {
            send(socketEnJeu, "Coup invalide", strlen("Coup invalide") + 1, 0);
            continue;
        }
        morpion->cases[x][y].symbole = symboleEnJeu;

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

        // Vérification des conditions de victoire ou de match nul
        if (verifierVictoire(morpion, symboleEnJeu)) {
            sprintf(messageEnvoyee, "%c gagne !", symboleEnJeu);
            send(clientX, messageEnvoyee, strlen(messageEnvoyee) + 1, 0);
            send(clientO, messageEnvoyee, strlen(messageEnvoyee) + 1, 0);
            break;
        }

        if (grillePleine(morpion)) {
            send(clientX, "Match nul", strlen("Match nul") + 1, 0);
            send(clientO, "Match nul", strlen("Match nul") + 1, 0);
            break;
        }

        // Changer de joueur
        if (clientEnJeu == 1) {
            clientEnJeu = 2;
        } else {
            clientEnJeu = 1;
        }
    }

    // Afficher la grille après le choix du serveur
    printf("Grille après le coup du serveur :\n");
    afficherGrille(morpion);

    close(clientX);
    close(clientO);
    close(socketEcoute);
    return 0;
}