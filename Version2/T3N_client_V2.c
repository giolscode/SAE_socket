#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "Grille.h"

#define PORT 6000
#define LG_MESSAGE 256

int main(int argc, char *argv[]) {

    int descripteurSocket;
    struct sockaddr_in sockaddrDistant;
    socklen_t longueurAdresse;

    char messageRecu[LG_MESSAGE]; // Client <-- Serveur
    char messageEnvoye[LG_MESSAGE]; // Client --> Serveur
    char symboleJoueur = argv[3][0]; // X ou O
    Grille *morpion = creerGrille(3, 3);
    int cln, lgn;

    // si il n'y a pas 4 arguments lors de l'éxecution du code comme 
    // par exemple (./T3N_client_V2 127.0.0.1 6000 X) sa ne lanceras pas le client.
    if (argc < 4) {
        printf("USAGE : %s ip port X ou O\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    // Création du socket
    descripteurSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (descripteurSocket < 0) {
        perror("Erreur de création de la socket");
        exit(EXIT_FAILURE);
    }
    printf("Socket créée! (%d) \n", descripteurSocket);

    //Remplissage du sockaddrDistant
    longueurAdresse = sizeof(sockaddrDistant);
    memset(&sockaddrDistant, 0, longueurAdresse);
    sockaddrDistant.sin_family = AF_INET;
    sockaddrDistant.sin_addr.s_addr = htonl(INADDR_ANY);
    sockaddrDistant.sin_port = htons(PORT);

    // Tentative de connexion du client au serveur
    if (connect(descripteurSocket, (struct sockaddr *)&sockaddrDistant, sizeof(sockaddrDistant)) == -1) {
        perror("Erreur de connexion");
        close(descripteurSocket);
        exit(EXIT_FAILURE);
    }
    printf("Connexion établie avec le serveur.\n");

    while (1) {

        //Réception du message du serveur
        memset(messageRecu, 0, LG_MESSAGE);
        int recu = read(descripteurSocket, messageRecu, LG_MESSAGE);
        if (recu <= 0) {
            perror("Erreur lors de la réception du message");
            break;
        }

        if (strcmp(messageRecu, "your_turn") == 0) {

            afficherGrille(morpion);
            
            // Le Joueur a qui sait le tour joue
            printf("Votre tour (%c) , entrez votre coup comme ceci (ex : 1 2) : ", symboleJoueur);
            scanf("%d %d", &lgn, &cln);
            sprintf(messageEnvoye, "%d %d", lgn, cln);
            write(descripteurSocket,messageEnvoye,strlen(messageEnvoye)+1);
        } 
        else if (strcmp(messageRecu, "wait") == 0) {
            printf("En attente de l'autre joueur...\n");
        }
        // Dans le cas ou l'un des deux joueurs gagne la partie 
        else if (strcmp(messageRecu, "Xwins") == 0 || strcmp(messageRecu, "Owins") == 0) {
            printf("Le joueur %c a gagné !\n", messageRecu[0]);
            afficherGrille(morpion);
            break;
        } 
        // Dans le cas ou la grille est pleine
        else if (strcmp(messageRecu, "Xend") == 0 || strcmp(messageRecu, "Oend") == 0) {
            printf("Grille pleine. Pas de gagnant.\n");
            afficherGrille(morpion);
            break;
        } 
        // Dans le cas ou le coup est pas permit
        else if (strcmp(messageRecu, "invalid") == 0) {
            printf("Coup invalide. Réessayez.\n");
        } 
        // Si le serveur envoie un message autre que ceux prévut
        else {
            printf("Message inconnu reçu : %s\n", messageRecu);
        }
    }

    //fermeture de la session
    libererGrille(morpion);
    close(descripteurSocket);
    
    return 0;
}
