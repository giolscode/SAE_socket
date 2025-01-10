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

    char messageRecu[LG_MESSAGE];
    char messageEnvoye[LG_MESSAGE];
    Grille *morpion = creerGrille(3, 3);

    if (argc < 4) {
        printf("USAGE : %s ip port X ou O\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    char symboleJoueur = argv[3][0];
    descripteurSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (descripteurSocket < 0) {
        perror("Erreur de création de la socket");
        exit(EXIT_FAILURE);
    }

    longueurAdresse = sizeof(sockaddrDistant);
    memset(&sockaddrDistant, 0, longueurAdresse);
    sockaddrDistant.sin_family = AF_INET;
    sockaddrDistant.sin_port = htons(atoi(argv[2]));
    inet_aton(argv[1], &sockaddrDistant.sin_addr);

    if (connect(descripteurSocket, (struct sockaddr *)&sockaddrDistant, sizeof(sockaddrDistant)) == -1) {
        perror("Erreur de connexion");
        close(descripteurSocket);
        exit(EXIT_FAILURE);
    }

    printf("Connexion établie avec le serveur.\n");

    while (1) {
        memset(messageRecu, 0, LG_MESSAGE);
        int recu = read(descripteurSocket, messageRecu, LG_MESSAGE);
        if (recu <= 0) {
            perror("Erreur lors de la réception du message");
            break;
        }

        if (strcmp(messageRecu, "your_turn") == 0) {
            afficherGrille(morpion);
            printf("Votre tour (%c) : ", symboleJoueur);
            int x, y;
            scanf("%d %d", &x, &y);
            sprintf(messageEnvoye, "%d %d", x, y);
            if (send(descripteurSocket, messageEnvoye, strlen(messageEnvoye) + 1, 0) <= 0) {
                perror("Erreur lors de l'envoi du coup");
                break;
            }
        } else if (strcmp(messageRecu, "wait") == 0) {
            printf("En attente de l'autre joueur...\n");
        } else if (strcmp(messageRecu, "Xwins") == 0 || strcmp(messageRecu, "Owins") == 0) {
            printf("Le joueur %c a gagné !\n", messageRecu[0]);
            afficherGrille(morpion);
            break;
        } else if (strcmp(messageRecu, "Xend") == 0 || strcmp(messageRecu, "Oend") == 0) {
            printf("Grille pleine. Pas de gagnant.\n");
            afficherGrille(morpion);
            break;
        } else if (strcmp(messageRecu, "invalid") == 0) {
            printf("Coup invalide. Réessayez.\n");
        } else {
            printf("Message inconnu reçu : %s\n", messageRecu);
        }
    }

    libererGrille(morpion);
    close(descripteurSocket);
    return 0;
}
