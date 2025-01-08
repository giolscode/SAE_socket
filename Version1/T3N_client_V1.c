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


void traiterAction(const char *action, int caseServeur, Grille *morpion) {

    if (strcmp(action, "continue") == 0) {
        printf("Le serveur a joué à la case %d. La partie continue.\n", caseServeur);
    } 
    else if (strcmp(action, "Owins") == 0) {
        printf("Le serveur a joué à la case %d. Le serveur (O) a gagné !\n", caseServeur);
        afficherGrille(morpion);
        // Fin de la partie, quitter la boucle
        return 0;
    } 
    else if (strcmp(action, "Oend") == 0) {
        printf("Le serveur a joué à la case %d. Grille pleine, pas de gagnant.\n", caseServeur);
        afficherGrille(morpion);
        // Fin de la partie, quitter la boucle
        return 0;
    } 
    else if (strcmp(action, "Xwins") == 0) {
        printf("Félicitations ! Vous avez gagné !\n");
        afficherGrille(morpion);
        // Fin de la partie, quitter la boucle
        return 0;
    } 
    else if (strcmp(action, "Xend") == 0) {
        printf("Grille pleine, pas de gagnant. La partie est terminée.\n");
        afficherGrille(morpion);
        // Fin de la partie, quitter la boucle
        return 0;
    } 
    else {
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

    // Initialiser la grille
    morpion = creerGrille(3, 3);

    while (1) {
        afficherGrille(morpion);
        printf("Quelle case voulez-vous choisir ? (Ligne colonne, séparées par un espace)\n");

        if (scanf("%d %d", &lgn, &cln) != 2) {
            printf("Erreur : Il faut saisir la ligne et la colonne dans la grille séparées par un espace !\n");
            continue;
        }

        int ligne = lgn - 1;
        int colonne = cln - 1;

        if (ligne < 0 || ligne >= 3 || colonne < 0 || colonne >= 3 || morpion->cases[ligne][colonne].symbole != ' ') {
            printf("Erreur : Case invalide ou déjà occupée !\n");
            continue;
        }

        morpion->cases[ligne][colonne].symbole = 'X';

        // Envoyer le coup au serveur
        snprintf(messageRecu, LG_MESSAGE, "%d %d", ligne, colonne);
        nb = write(descripteurSocket, messageRecu, strlen(messageRecu));
        if (nb <= 0) {
            perror("Erreur lors de l'envoi des données...");
            break;
        }

        // Attendre la réponse du serveur
        nb = read(descripteurSocket, messageRecu, LG_MESSAGE);
        if (nb <= 0) {
            perror("Erreur lors de la réception des données...");
            break;
        }

        messageRecu[nb] = '\0';

        // Gestion des messages du serveur
        char action[10];
        int caseServeur;

        sscanf(messageRecu, "%s %d", action, &caseServeur);

        int x = (caseServeur - 1) / 3;
        int y = (caseServeur - 1) % 3;

        if (caseServeur >= 1 && caseServeur <= 9) {
            morpion->cases[x][y].symbole = 'O';
        }

        traiterAction(action,caseServeur,morpion);

    }

    libererGrille(morpion); 
    close(descripteurSocket);
    return 0;
}