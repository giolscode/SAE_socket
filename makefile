# Compilation de Grille.c
Grille.o:
	gcc -c Grille.c -o Grille.o
	gcc T3N_serveur_Vx.c Grille.o -o T3N_serveur_Vx
	gcc T3N_client_Vx.c Grille.o -o T3N_client_Vx

#Lancer le serveur
serveur : 
	./T3N_serveur_Vx

# Lancer le client
client : 
	./T3N_client_Vx 127.0.0.1 6000

# Nettoyage des fichiers générés
clean:
	rm -f Grille.o T3N_serveur_Vx T3N_client_Vx

