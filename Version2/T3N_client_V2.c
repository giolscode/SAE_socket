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
        printf("Le joueur O a gagné !\n");
        afficherGrille(morpion);
    } 
    else if (strcmp(action, "Oend") == 0) {
        printf("Grille pleine, pas de gagnant.\n");
        afficherGrille(morpion);
    } 
    else if (strcmp(action, "Xwins") == 0) {
        printf("Le joueur X a gagné !\n");
        afficherGrille(morpion);
    } 
    else if (strcmp(action, "Xend") == 0) {
        printf("Grille pleine, pas de gagnant.\n");
        afficherGrille(morpion);
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
    char roleJoueur;

    // Vérification des arguments pour récupérer l'IP et le port
    if (argc > 2) {
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

    // Recevoir le rôle du joueur
    nb = read(descripteurSocket, &roleJoueur, sizeof(char));
    if (nb <= 0) {
        perror("Erreur lors de la réception du rôle...");
        close(descripteurSocket);
        exit(-3);
    }
    printf("Votre rôle : %c\n", roleJoueur);

    // Initialiser la grille
    morpion = creerGrille(3, 3);

    while (1) {
        // Attendre un message du serveur pour savoir si c'est au tour du client de jouer
        nb = read(descripteurSocket, messageRecu, LG_MESSAGE);
        if (nb <= 0) {
            perror("Erreur lors de la réception des données...");
            break;
        }

        messageRecu[nb] = '\0';  // Assurer que le message est terminé
        printf("Message reçu : %s\n", messageRecu);

        // Si le serveur envoie un message pour une action, effectuer la mise à jour de la grille
        char action[10];
        int caseServeur = -1;
        int scanResult = sscanf(messageRecu, "%s %d", action, &caseServeur);

        if (scanResult >= 1) {
            if (caseServeur > 0) {
                int x = (caseServeur - 1) / 3;
                int y = (caseServeur - 1) % 3;
                morpion->cases[x][y].symbole = (roleJoueur == 'X') ? 'O' : 'X';
            }
            
            traiterAction(action, caseServeur, morpion);

            if (strcmp(action, "Owins") == 0 || strcmp(action, "Xwins") == 0 || strcmp(action, "Oend") == 0 || strcmp(action, "Xend") == 0) {
                printf("Fin de la partie. Merci d'avoir joué !\n");
                break;
            }
        } else {
            printf("Message inconnu ou mal formé : %s\n", messageRecu);
        }

        // Jouer un coup
        afficherGrille(morpion);
        printf("Quelle case voulez-vous choisir ? (Ligne colonne, séparées par un espace)\n");

        if (scanf("%d %d", &lgn, &cln) != 2) {
            printf("Erreur : Il faut saisir la ligne et la colonne séparées par un espace !\n");
            continue;
        }

        int ligne = lgn - 1;
        int colonne = cln - 1;

        if (ligne < 0 || ligne >= 3 || colonne < 0 || colonne >= 3 || morpion->cases[ligne][colonne].symbole != ' ') {
            printf("Erreur : Case invalide ou déjà occupée !\n");
            continue;
        }

        morpion->cases[ligne][colonne].symbole = roleJoueur;

        snprintf(messageRecu, LG_MESSAGE, "%d %d", ligne, colonne);
        nb = write(descripteurSocket, messageRecu, strlen(messageRecu));
        if (nb <= 0) {
            perror("Erreur lors de l'envoi des données...");
            break;
        }
    }

    libererGrille(morpion);
    close(descripteurSocket);
    return 0;
}