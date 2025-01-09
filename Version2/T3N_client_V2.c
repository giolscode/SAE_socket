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

void traiterAction(const char *action, Grille *morpion, char symboleEnJeu) {
    if (strcmp(action, "continue") == 0) {
        printf("La partie continue. À l'autre joueur de jouer.\n");
    } else if (strcmp(action, "Xwins") == 0 || strcmp(action, "Owins") == 0) {
        printf("Le joueur %c a gagné !\n", symboleEnJeu);
        afficherGrille(morpion);
    } else if (strcmp(action, "Xend") == 0 || strcmp(action, "Oend") == 0) {
        printf("Grille pleine. Pas de gagnant.\n");
        afficherGrille(morpion);
    } else {
        printf("Message inconnu reçu : %s\n", action);
    }
}

int main(int argc, char *argv[]) {
    int descripteurSocket;
    struct sockaddr_in sockaddrDistant;
    socklen_t longueurAdresse;

    Grille *morpion;
    int lgn, cln;
    char messageRecu[LG_MESSAGE];
    char messageEnvoye[LG_MESSAGE];
    char symboleJoueur;

    // Vérification des arguments
    if (argc < 4) {
        printf("USAGE : %s ip port X ou O \n", argv[0]);
        exit(-1);
    }

    symboleJoueur = argv[3][0];
    if (symboleJoueur != 'X' && symboleJoueur != 'O') {
        printf("Erreur : Vous devez choisir X ou O comme symbole.\n");
        exit(-1);
    }

    // Création de la socket
    descripteurSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (descripteurSocket < 0) {
        perror("Erreur en création de la socket...");
        exit(-1);
    }

    // Configuration de l'adresse du serveur
    longueurAdresse = sizeof(sockaddrDistant);
    memset(&sockaddrDistant, 0, longueurAdresse);
    sockaddrDistant.sin_family = AF_INET;
    sockaddrDistant.sin_port = htons(atoi(argv[2]));
    inet_aton(argv[1], &sockaddrDistant.sin_addr);

    // Connexion au serveur
    if (connect(descripteurSocket, (struct sockaddr *)&sockaddrDistant, longueurAdresse) == -1) {
        perror("Erreur de connexion avec le serveur...");
        close(descripteurSocket);
        exit(-2);
    }
    printf("Connexion réussie au serveur %s:%s\n", argv[1], argv[2]);

    // Initialiser la grille
    morpion = creerGrille(3, 3);

    while (1) {
        afficherGrille(morpion);

        if (symboleJoueur == 'X') {
            printf("Joueur %c, entrez votre coup (ligne colonne) : ", symboleJoueur);
            scanf("%d %d", &lgn, &cln);

            // Validation du coup
            int ligne = lgn - 1;
            int colonne = cln - 1;
            if (ligne < 0 || ligne >= 3 || colonne < 0 || colonne >= 3 || morpion->cases[ligne][colonne].symbole != ' ') {
                printf("Coup invalide. Réessayez.\n");
                continue;
            }

            // Marquer la case localement
            morpion->cases[ligne][colonne].symbole = symboleJoueur;

            // Envoyer le coup au serveur
            snprintf(messageEnvoye, LG_MESSAGE, "%d %d", ligne, colonne);
            if (write(descripteurSocket, messageEnvoye, strlen(messageEnvoye)) <= 0) {
                perror("Erreur lors de l'envoi du coup.");
                break;
            }
        }

        // Recevoir le résultat du serveur
        memset(messageRecu, 0, LG_MESSAGE);
        int nb = read(descripteurSocket, messageRecu, LG_MESSAGE);
        if (nb <= 0) {
            perror("Erreur lors de la réception.");
            break;
        }

        messageRecu[nb] = '\0';  // Assurer une chaîne valide

        // Mise à jour de la grille selon le message
        char action[10];
        int caseServeur;
        sscanf(messageRecu, "%s %d", action, &caseServeur);

        if (caseServeur > 0) {
            int x = (caseServeur - 1) / 3;
            int y = (caseServeur - 1) % 3;
            morpion->cases[x][y].symbole = (symboleJoueur == 'X') ? 'O' : 'X';
        }

        // Traiter le résultat reçu
        traiterAction(action, morpion, symboleJoueur);

        // Vérifier la fin du jeu
        if (strcmp(action, "Xwins") == 0 || strcmp(action, "Owins") == 0 || strcmp(action, "Xend") == 0 || strcmp(action, "Oend") == 0) {
            break;
        }
    }

    // Libération des ressources
    libererGrille(morpion);
    close(descripteurSocket);
    return 0;
}