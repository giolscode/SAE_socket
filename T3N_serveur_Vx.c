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
    int x,y,numCase;
    int longueur = 3;
    int largeur = 3;

	struct sockaddr_in pointDeRencontreLocal;
	socklen_t longueurAdresse;

	int socketDialogue;
	struct sockaddr_in pointDeRencontreDistant;
	char messageRecu[LG_MESSAGE]; /* le message de la couche Application ! */
	int ecrits, lus; /* nb d’octets ecrits et lus */
	int retour;

    //Création du socket
    socketEcoute = socket(AF_INET, SOCK_STREAM, 0); 
    if (socketEcoute < 0){
        perror("socket");
        exit(-1);
    }
    printf("Socket créée avec succès ! (%d)\n", socketEcoute);

    // Remplissage de sockaddrDistant (structure sockaddr_in identifiant le point d'écoute local)
	longueurAdresse = sizeof(pointDeRencontreLocal);
	// memset sert à faire une copie d'un octet n fois à partir d'une adresse mémoire donnée
	// ici l'octet 0 est recopié longueurAdresse fois à partir de l'adresse &pointDeRencontreLocal
	memset(&pointDeRencontreLocal, 0x00, longueurAdresse); pointDeRencontreLocal.sin_family = PF_INET;
	pointDeRencontreLocal.sin_addr.s_addr = htonl(INADDR_ANY); // attaché à toutes les interfaces locales disponibles
	pointDeRencontreLocal.sin_port = htons(PORT); // = 5000 ou plus

    // On demande l’attachement local de la socket
	if((bind(socketEcoute, (struct sockaddr *)&pointDeRencontreLocal, longueurAdresse)) < 0) {
		perror("bind");
		exit(-2); 
	}
	printf("Socket attachée avec succès !\n");

    //mise en écoute
    if(listen(socketEcoute, 5) < 0){
   		perror("listen");
   		exit(-3);
	}
	printf("Socket placée en écoute passive ... \n \n");

    //envoie du message start
    send(socketEcoute,"start",strlen("start")+1,0);

    //initialisation de la grille
    morpion = creerGrille(longueur,largeur);
    afficherGrille(morpion);

    // boucle d’attente de connexion : en théorie, un serveur attend indéfiniment ! 
	while(1){
		memset(messageRecu, 'a', LG_MESSAGE*sizeof(char));
		printf("Attente d’une demande de connexion (quitter avec Ctrl-C)\n\n");
		
		// c’est un appel bloquant
		socketDialogue = accept(socketEcoute, (struct sockaddr *)&pointDeRencontreDistant, &longueurAdresse);
		if (socketDialogue < 0) {
   			perror("accept");
			close(socketDialogue);
   			close(socketEcoute);
   			exit(-4);
		}
		
		// On réception les données du client (cf. protocole)
		lus = recv(socketDialogue, messageRecu, LG_MESSAGE*sizeof(char),0); // ici appel bloquant
		switch(lus) {
			case -1 : /* une erreur ! */ 
				  perror("read"); 
				  close(socketDialogue); 
				  exit(-5);
			case 0  : /* la socket est fermée */
				  fprintf(stderr, "La socket a été fermée par le client !\n\n");
   				  close(socketDialogue);
   				  return 0;
			default:  /* réception de n octets */
				  printf("Message reçu : %s (%d octets)\n\n", messageRecu, lus);
		}

        // Interprétation des coordonnées jouées par le client
        int numCase = messageRecu[0] - '0' ; 
        int x = (numCase - 1) / largeur ; //Ligne
        int y = (numCase - 1) % longueur ;  //colonne

        if (numCase >= 1 && numCase <= longueur * largeur && morpion->cases[x][y].symbole == ' '){
            morpion->cases[x][y].symbole = 'X' ; 
        }
        else{
            perror("CASE OCCUPÉE");
        }

        // Afficher la grille mise à jour
        printf("Grille après le coup du client :\n");
        afficherGrille(morpion);

        int caseServeur;
        
        do {
            caseServeur = rand() % (largeur * longueur) + 1; // Nombre aléatoire entre 1 et 9
            x = (caseServeur - 1) / largeur; // Ligne
            y = (caseServeur - 1) % longueur; // Colonne
        } while (morpion->cases[x][y].symbole != ' '); // Répéter tant que la case est occupée

        morpion->cases[x][y].symbole = 'O'; // Serveur joue

        // Envoyer la case choisie au client
        char caseEnvoyee[3];
        snprintf(caseEnvoyee, sizeof(caseEnvoyee), "%d", caseServeur);
        send(socketDialogue, caseEnvoyee, strlen(caseEnvoyee) + 1, 0);

        // Afficher la grille après le choix du serveur
        printf("Grille après le coup du serveur :\n");
        afficherGrille(morpion);

	}
    
    close(socketEcoute);
    return 0;
}
