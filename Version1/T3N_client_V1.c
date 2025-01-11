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

#define LG_MESSAGE 256

/// @brief Fonction qui gére les actions recu de la part du serveur et permet de traiter celle ci.
/// @param action Représente l'action reçue (ex : "continue", "Owins", etc.).
/// @param caseServeur C'est la case joué par le serveur (index de la grille).
/// @param morpion Pointeur vers la grille du jeu (structure Grille).
void traiterAction(const char *action, int caseServeur, Grille *morpion) {
    if (strcmp(action, "continue") == 0) {
        printf("Le serveur a joué à la case %d. La partie continue.\n", caseServeur);
    } 
    else if (strcmp(action, "Owins") == 0) {
        printf("Le serveur a joué à la case %d. Le serveur (O) a gagné !\n", caseServeur);
        afficherGrille(morpion);
    } 
    else if (strcmp(action, "Oend") == 0) {
        printf("Le serveur a joué à la case %d. Grille pleine, pas de gagnant.\n", caseServeur);
        afficherGrille(morpion);
    } 
    else if (strcmp(action, "Xwins") == 0) {
        printf("Félicitations ! Vous avez gagné !\n");
        afficherGrille(morpion);
    } 
    else if (strcmp(action, "Xend") == 0) {
        printf("Grille pleine, pas de gagnant. La partie est terminée.\n");
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
    // les lignes et les colonnes du morpions 
    int lgn, cln;

    char buffer[LG_MESSAGE];
    char messageRecu[LG_MESSAGE]; 
    int nb; /* nb d’octets écrits et lus */

    char ip_dest[16];
    int port_dest;

    // Gérer les messages du serveur
    char action[10]; 
    int caseServeur;

    // lire une action et une case 
    int scanResult;

    // coordonnées 
    int x, y;

    // Pour pouvoir contacter le serveur, le client doit connaître son adresse IP et le port de comunication
	// Ces 2 informations sont passées sur la ligne de commande
	// Si le serveur et le client tournent sur la même machine alors l'IP locale fonctionne : 127.0.0.1
	// Le port d'écoute du serveur est 6000 dans cet exemple, donc en local utiliser la commande :
	// ./client_base_tcp 127.0.0.1 6000
    if (argc > 1) { // si il y a au moins 2 arguments passés en ligne de commande, récupération ip et port
        strncpy(ip_dest, argv[1], 16);
        sscanf(argv[2], "%d", &port_dest);
    } else {
        printf("USAGE : %s ip port\n", argv[0]);
        exit(-1);
    }

    // Crée un socket de communication
    descripteurSocket = socket(AF_INET, SOCK_STREAM, 0);
    // Teste la valeur renvoyée par l’appel système socket()
    if (descripteurSocket < 0) {
        perror("Erreur en création de la socket..."); // Affiche le message d’erreur
        exit(-1); // On sort en indiquant un code erreur
    }
    printf("Socket créée! (%d)\n", descripteurSocket);

    // Remplissage de sockaddrDistant (structure sockaddr_in identifiant la machine distante)
	// Obtient la longueur en octets de la structure sockaddr_in
    longueurAdresse = sizeof(sockaddrDistant);
    // Initialise à 0 la structure sockaddr_in
	// memset sert à faire une copie d'un octet n fois à partir d'une adresse mémoire donnée
	// ici l'octet 0 est recopié longueurAdresse fois à partir de l'adresse &sockaddrDistant
    memset(&sockaddrDistant, 0x00, longueurAdresse);
    // Renseigne la structure sockaddr_in avec les informations du serveur distant
    sockaddrDistant.sin_family = AF_INET;
    // On choisit le numéro de port d’écoute du serveur
    sockaddrDistant.sin_port = htons(port_dest);
    // On choisit l’adresse IPv4 du serveur
    inet_aton(ip_dest, &sockaddrDistant.sin_addr);

    // Débute la connexion vers le processus serveur distant
    if ((connect(descripteurSocket, (struct sockaddr *)&sockaddrDistant, longueurAdresse)) == -1) {
        perror("Erreur de connection avec le serveur distant...");
        close(descripteurSocket);
        exit(-2); // On sort en indiquant un code erreur
    }
    printf("Connexion au serveur %s:%d réussie!\n", ip_dest, port_dest);

    // On initialise notre grille  
    morpion = creerGrille(3, 3);

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

        // On affiche la grille et on demande au client la case choisi 
        afficherGrille(morpion);
        printf("Quelle case voulez-vous choisir ? (Ligne colonne, séparées par un espace)\n");

        if (scanf("%d %d", &lgn, &cln) != 2) {
            printf("Erreur : Il faut saisir la ligne et la colonne dans la grille séparées par un espace !\n");
            continue;
        }

        int ligne = lgn - 1, colonne = cln - 1;
        if (ligne < 0 || ligne >= 3 || colonne < 0 || colonne >= 3 || morpion->cases[ligne][colonne].symbole != ' ') {
            printf("Erreur : Case invalide ou déjà occupée !\n");
            continue;
        }

        morpion->cases[ligne][colonne].symbole = 'X';

        // on envoie le coup au serveur
        snprintf(messageRecu, LG_MESSAGE, "%d %d", ligne, colonne);
        nb = write(descripteurSocket, messageRecu, strlen(messageRecu));
        if (nb <= 0) {
            perror("Erreur lors de l'envoi des données...");
            break;
        }

        // attend la réponse du serv
        nb = read(descripteurSocket, messageRecu, LG_MESSAGE);
        if (nb <= 0) {
            perror("Erreur lors de la réception des données...");
            break;
        }

        messageRecu[nb] = '\0';  // on s'assure que le mess est terminé 
        
        // Essayer de lire une action et une case
        int scanResult = sscanf(messageRecu, "%s %d", action, &caseServeur);

        if (scanResult == 2) {
            // contient l'action et la case
            printf("Action : %s, Case : %d\n", action, caseServeur);
            int x = (caseServeur - 1) / 3;
            int y = (caseServeur - 1) % 3;
            morpion->cases[x][y].symbole = 'O';
        } else if (scanResult == 1) {
            // que le numéro de la case
            sscanf(messageRecu, "%d", &caseServeur);
            int x = (caseServeur - 1) / 3;
            int y = (caseServeur - 1) % 3;
            morpion->cases[x][y].symbole = 'O';
        } else {
            printf("Message mal formé ou inconnu : %s\n", messageRecu);
        }        

        // On traite l'action reçue
        traiterAction(action, caseServeur, morpion);

        if (strcmp(action, "Owins") == 0 || strcmp(action, "Xwins") == 0 || strcmp(action, "Oend") == 0) {
            // On libére la mémoire de la grille 
            libererGrille(morpion);
            // On ferme la ressource avant de quitter
            close(descripteurSocket);
            exit(0);
        }

        
    }

    // On libére la mémoire de la grille 
    libererGrille(morpion); 
    // On ferme la ressource avant de quitter
    close(descripteurSocket);
    return 0;
}