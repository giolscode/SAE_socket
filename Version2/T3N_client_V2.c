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
#include "Grille.h" /* Class Grille */

#define PORT 6000
#define LG_MESSAGE 256

int main(int argc, char *argv[]) {

    int descripteurSocket;
    struct sockaddr_in sockaddrDistant;
    socklen_t longueurAdresse;

    char messageRecu[LG_MESSAGE]; // Client <-- Serveur
    char messageEnvoye[LG_MESSAGE]; // Client --> Serveur
    char symboleJoueur = argv[3][0]; // X ou O
    // Création de notre grille 
    Grille *morpion = creerGrille(3, 3);
    // les lignes et les colonnes du morpions 
    int cln, lgn;

    // si il n'y a pas 4 arguments lors de l'éxecution du code comme 
    // par exemple (./T3N_client_V2 127.0.0.1 6000 X) sa ne lanceras pas le client.
    if (argc < 4) {
        printf("USAGE : %s ip port X ou O\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    // Crée un socket de communication
    descripteurSocket = socket(AF_INET, SOCK_STREAM, 0);
    // Teste la valeur renvoyée par l’appel système socket()
    if (descripteurSocket < 0) {
        perror("Erreur de création de la socket"); // Affiche le message d’erreur
        exit(EXIT_FAILURE); // On sort en indiquant un code erreur
    }
    printf("Socket créée! (%d) \n", descripteurSocket);

    // Remplissage de sockaddrDistant (structure sockaddr_in identifiant la machine distante)
	// Obtient la longueur en octets de la structure sockaddr_in
    longueurAdresse = sizeof(sockaddrDistant);
    // Initialise à 0 la structure sockaddr_in
	// memset sert à faire une copie d'un octet n fois à partir d'une adresse mémoire donnée
	// ici l'octet 0 est recopié longueurAdresse fois à partir de l'adresse &sockaddrDistant
    memset(&sockaddrDistant, 0, longueurAdresse);
    // Renseigne la structure sockaddr_in avec les informations du serveur distant
    sockaddrDistant.sin_family = AF_INET;
    // On choisit le numéro de port d’écoute du serveur
    sockaddrDistant.sin_addr.s_addr = htonl(INADDR_ANY);
    // On choisit l’adresse IPv4 du serveur
    sockaddrDistant.sin_port = htons(PORT);

    // Débute la connexion vers le processus serveur distant
    if (connect(descripteurSocket, (struct sockaddr *)&sockaddrDistant, sizeof(sockaddrDistant)) == -1) {
        perror("Erreur de connection avec le serveur distant...");
        close(descripteurSocket);
        exit(EXIT_FAILURE); // On sort en indiquant un code erreur
    }
    printf("Connexion au serveur %s:%d réussie!\n", ip_dest, port_dest);

    while (1) {
        // Envoi du message
        //switch(nb = write(descripteurSocket, buffer, strlen(buffer))){
        switch(nb = send(descripteurSocket, buffer, strlen(buffer)+1,0)){
            case -1 : /* une erreur ! */
                    perror("Erreur en écriture...");
                    close(descripteurSocket);
                    exit(-3);
            case 0 : /* le socket est fermée */
                fprintf(stderr, "Le socket a été fermée par le serveur !\n\n");
                return 0;
            default: /* envoi de n octets */
                printf("Message %s envoyé! (%d octets)\n\n", buffer, nb);
        }

        //Réception du message du serveur
        memset(messageRecu, 0, LG_MESSAGE);
        int recu = read(descripteurSocket, messageRecu, LG_MESSAGE);
        if (recu <= 0) {
            perror("Erreur lors de la réception du message");
            break;
        }

        if (strcmp(messageRecu, "continue") == 0) {

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

    // On libére la mémoire de la grille 
    libererGrille(morpion);
    // On ferme la ressource avant de quitter
    close(descripteurSocket);
    
    return 0;
}
