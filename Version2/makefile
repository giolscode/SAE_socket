# Compilation de Grille.c
Grille.o:
	gcc -c Grille.c -o Grille.o
	gcc T3N_serveur_V2.c Grille.o -o T3N_serveur_V2
	gcc T3N_client_V2.c Grille.o -o T3N_client_V2

#Lancer le serveur
serveur : 
	./T3N_serveur_V2

# Lancer le client
client : 
	./T3N_client_V2 127.0.0.1 6000

# Nettoyage des fichiers générés
clean:
	rm -f Grille.o T3N_serveur_V2 T3N_client_V2

