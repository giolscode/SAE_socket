#include <stdio.h>
#include <stdlib.h> /* pour exit */
#include <unistd.h> /* pour read, write, close, sleep */
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h> /* pour memset */
#include <netinet/in.h> /* pour struct sockaddr_in */
#include <arpa/inet.h> /* pour htons et inet_aton */
#include "Grille.h"

#define PORT 6000 
#define LG_MESSAGE 256

int main(int argc, char const *argv[])
{
    int socketEcoute;
    Grille *morpion;
    int x, y, numCase;
    int longueur = 3;
    int largeur = 3;

    struct sockaddr_in pointDeRencontreLocal;
    socklen_t longueurAdresse;

    int socketDialogue;
    struct sockaddr_in pointDeRencontreDistant;
    char messageRecu[LG_MESSAGE]; /* le message de la couche Application ! */
    int ecrits, lus; /* nb d’octets ecrits et lus */
    int retour;

    // Création du socket
    socketEcoute = socket(AF_INET, SOCK_STREAM, 0); 
    if (socketEcoute < 0){
        perror("socket");
        exit(-1);
    }
    printf("Socket créée avec succès ! (%d)\n", socketEcoute);

    // Remplissage de sockaddrDistant (structure sockaddr_in identifiant le point d'écoute local)
    longueurAdresse = sizeof(pointDeRencontreLocal);
    memset(&pointDeRencontreLocal, 0x00, longueurAdresse); pointDeRencontreLocal.sin_family = PF_INET;
    pointDeRencontreLocal.sin_addr.s_addr = htonl(INADDR_ANY); // attaché à toutes les interfaces locales disponibles
    pointDeRencontreLocal.sin_port = htons(PORT); // port 6000

    // On demande l’attachement local de la socket
    if ((bind(socketEcoute, (struct sockaddr *)&pointDeRencontreLocal, longueurAdresse)) < 0) {
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

    // attente de connexion 
    printf("Attente d’une demande de connexion...\n\n");
    socketDialogue = accept(socketEcoute, (struct sockaddr *)&pointDeRencontreDistant, &longueurAdresse);
    if (socketDialogue < 0) {
        perror("accept");
        close(socketDialogue);
        close(socketEcoute);
        exit(-4);
    }
    printf("Connexion acceptée. Le jeu commence !\n");

    // Réinitialisation du message pour chaque tour
    while (1) {
        memset(messageRecu, 0, LG_MESSAGE); // Réinitialisation du message

        // On réceptionne les données du client (cf. protocole)
        lus = recv(socketDialogue, messageRecu, LG_MESSAGE, 0); // ici appel bloquant
        switch (lus) {
            case -1: /* une erreur */
                perror("read");
                close(socketDialogue);
                exit(-5);
            case 0:  /* la socket est fermée */
                fprintf(stderr, "La socket a été fermée par le client !\n\n");
                close(socketDialogue);
                return 0;
            default:  /* réception de n octets */
                messageRecu[lus] = '\0';  // Terminer le message par un caractère nul
                printf("Message reçu : '%s' (%d octets)\n\n", messageRecu, lus);
        }

        // Interprétation des coordonnées (x et y)
        if (sscanf(messageRecu, "%d %d", &x, &y) != 2) {
            printf("Erreur : coordonnées invalides reçues : %s\n", messageRecu);
            continue;
        }

        // Vérifier que les coordonnées sont dans les limites de la grille et que la case est vide
        if (x >= 0 && x < longueur && y >= 0 && y < largeur && morpion->cases[x][y].symbole == ' ') {
            morpion->cases[x][y].symbole = 'X'; // Client joue
        } else {
            printf("Case invalide ou déjà occupée : (%d, %d).\n", x, y);
            continue;
        }

        printf("Grille après le coup du client :\n");
        afficherGrille(morpion);

        // Tour du serveur de jouer
        int caseServeur;
        do {
            caseServeur = rand() % (longueur * largeur) + 1; // Entre 1 et 9
            x = (caseServeur - 1) / largeur; // Ligne
            y = (caseServeur - 1) % largeur; // Colonne
        } while (morpion->cases[x][y].symbole != ' ');

        // Placer le symbole du serveur
        morpion->cases[x][y].symbole = 'O';
        printf("Serveur joue à la case : %d (coordonnées: [%d][%d])\n", caseServeur, x, y);

        // Envoyer la case choisie au client
        char caseEnvoyee[3];
        sprintf(caseEnvoyee, "%d", caseServeur);
        if (send(socketDialogue, caseEnvoyee, strlen(caseEnvoyee) + 1, 0) <= 0) {
            perror("Erreur lors de l'envoi des données...");
            break;
        }

        // Afficher la grille après le choix du serveur
        printf("Grille après le coup du serveur :\n");
        afficherGrille(morpion);
    }

    close(socketEcoute);
    return 0;
}