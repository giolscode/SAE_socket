# Compilation de Grille.c
Grille.o:
	gcc -c Grille.c -o Grille.o
	gcc T3N_serveur_V3.c Grille.o -o T3N_serveur_V3
	gcc T3N_client_V3.c Grille.o -o T3N_client_V3

#Lancer le serveur
serveur : 
	./T3N_serveur_V3

# Lancer le client
clientX : 
	./T3N_client_V3 127.0.0.1 6000 X

clientO : 
	./T3N_client_V3 127.0.0.1 6000 O

# Ajouter une cible pour l'observateur
observe : 
	./T3N_client_V3 127.0.0.1 6000 observe

# Nettoyage des fichiers générés
clean:
	rm -f Grille.o T3N_serveur_V3 T3N_client_V3

