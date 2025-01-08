# Compilation de Grille.c
Grille.o:
	gcc -c Grille.c -o Grille.o
	gcc T3N_serveur_V1.c Grille.o -o T3N_serveur_V1
	gcc T3N_client_V1.c Grille.o -o T3N_client_V1

#Lancer le serveur
serveur : 
	./T3N_serveur_V1

# Lancer le client
client : 
	./T3N_client_V1 127.0.0.1 6000

# Nettoyage des fichiers générés
clean:
	rm -f Grille.o T3N_serveur_V1 T3N_client_V1

