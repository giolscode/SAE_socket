# Projet Sae Socket:

## Membres:

- Demol Alexis  
- Deburyne Lucas  
- Losat Giovanni  

```bash
gcc -c Grille.c -o Grille.o

gcc T3N_serveur_Vx.c Grille.o -o T3N_serveur_Vx

gcc T3N_client_Vx.c Grille.o -o T3N_client_Vx
```

## Pour la version 1 :

pré-requis :  
- posséder le C ainsi que toutes les bibliothèques   
- posséder makefile  
```bash
sudo apt install make
```

-> Lancer 2 terminales distincts  
-> Se déplacer dans le répertoire du dossier où se trouve les programmes pour les 2 terminales  
-> Faire un "make clean" sur l'un des terminal pour effacer toute trace de compilation passé  
-> Faire un "make" afin de compiler l'ensemble des programmes sur un terminal seulement  
-> Faire un "make serveur" pour lancer le serveur sur un terminal   
-> Faire un "make client" pour lancer un joueur sur un autre terminal (le client sera sur le port 6000 et non 5000)  
-> Donner des coordonnées tel que "1 2" sur le terminal client où "1" représente la ligne et "2" la colonne afin de pouvoir jouer   
-> Le serveur jouera ensuite et le programme s'arrête si l'un des deux joueurs gagne ou si il y a égalité   

## Pour la version 2 :

pré-requis :

- posséder le C ainsi que toutes les bibliothèques      
- posséder makefile  
```bash
sudo apt install make
````

-> Lancer 3 terminales distincts  
-> Se dépalcer dans le répertoire du dossier où se trouve les programmes pour les 3 terminales    
-> Faire un "make clean" sur l'un des terminal pour effacer toute trace de compilation passé  
-> Faire un "make" afin de compiler l'ensemble des programmes sur un terminal seulement   
-> Faire un "make serveur" pour lancer le serveur sur un terminal   
-> Faire un "make client" pour lancer un joueur sur un autre terminal (le client sera sur le port 6000 et non 5000)  
-> Faire de nouveau un "make client" pour lancer le second joueur sur le dernier terminal disponible (le client sera sur le port 6000 et non 5000)  
-> Donner des coordonnées tel que "1 2" sur l'un des terminal client où "1" représente la ligne et "2" la colonne afin de pouvoir jouer   
-> Donner des coordonnées avec la même syntaxe sur le second terminal client pour pouvoir jouer   
-> La partie se termine si l'un des deux joueurs gagne ou si il y a égalité  

