#include <stdio.h>
#include <stdlib.h> 
#include <unistd.h> 
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h> 
#include <netinet/in.h> 
#include <arpa/inet.h> 
#include "Grille.h"

#define LG_MESSAGE 256

int main(int argc, char *argv[]) {
    int descripteurSocket;
    
    struct sockaddr_in sockaddrDistant;
    socklen_t longueurAdresse;
    Grille *morpion;
    int lgn, cln;
    char buffer[LG_MESSAGE]; 
    int nb;

    char ip_dest[16];
    int port_dest;

    // Récupérer l'IP et le port du serveur
    if (argc > 1) {
        strncpy(ip_dest, argv[1], 16);
        sscanf(argv[2], "%d", &port_dest);
    } else {
        printf("USAGE : %s ip port\n", argv[0]);
        exit(-1);
    }

    // Créer le socket
    descripteurSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (descripteurSocket < 0) {
        perror("Erreur en création de la socket...");
        exit(-1); 
    }
    printf("Socket créée! (%d)\n", descripteurSocket);

    // Remplir sockaddrDistant
    longueurAdresse = sizeof(sockaddrDistant);
    memset(&sockaddrDistant, 0x00, longueurAdresse);
    sockaddrDistant.sin_family = AF_INET;
    sockaddrDistant.sin_port = htons(port_dest);
    inet_aton(ip_dest, &sockaddrDistant.sin_addr);

    // Connexion au serveur
    if ((connect(descripteurSocket, (struct sockaddr *)&sockaddrDistant, longueurAdresse)) == -1) {
        perror("Erreur de connexion avec le serveur...");
        close(descripteurSocket);
        exit(-2);
    }
    printf("Connexion au serveur %s:%d réussie!\n", ip_dest, port_dest);

    morpion = creerGrille(3, 3);

    while (1) {
        afficherGrille(morpion);
        printf("Quelle case voulez-vous choisir ? (Ligne colonne, séparée par un espace)\n");

        if (scanf("%d %d", &lgn, &cln) != 2) {
            printf("Erreur : Il faut saisir la ligne et la colonne dans la grille séparées par un espace !\n");
            continue;
        }

        int ligne = lgn - 1;
        int colonne = cln - 1;

        if (morpion->cases[ligne][colonne].symbole != ' ') {
            printf("Erreur : Cette case est déjà occupée !\n");
            continue;
        }

        morpion->cases[ligne][colonne].symbole = 'X';

        // Envoyer le coup au serveur
        snprintf(buffer, LG_MESSAGE, "%d %d", ligne, colonne);
        nb = write(descripteurSocket, buffer, strlen(buffer));
        if (nb <= 0) {
            perror("Erreur lors de l'envoi des données...");
            break;
        }

        // Attendre le coup du serveur
        nb = read(descripteurSocket, buffer, LG_MESSAGE);
        if (nb <= 0) {
            perror("Erreur lors de la réception des données...");
            break;
        }

        buffer[nb] = '\0';
        
        // Le serveur a joué
        int caseServeur;
        if (sscanf(buffer, "%d", &caseServeur) == 1) {
            int x = (caseServeur - 1) / 3;
            int y = (caseServeur - 1) % 3;
            morpion->cases[x][y].symbole = 'O';
            printf("Le serveur a joué à la case : %d (coordonnées : [%d][%d])\n", caseServeur, x, y);
        }
    }

    libererGrille(morpion); 
    close(descripteurSocket);
    return 0;
}