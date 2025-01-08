# Variables
CC = gcc
CFLAGS = -c
OBJ = Grille.o
SERVER_SRC = T3N_serveur_Vx.c
CLIENT_SRC = T3N_client_Vx.c
SERVER_BIN = T3N_serveur_Vx
CLIENT_BIN = T3N_client_Vx

# Règle par défaut
all: $(SERVER_BIN) $(CLIENT_BIN)

# Compilation de Grille.c
$(OBJ): Grille.c
	$(CC) $(CFLAGS) Grille.c -o $(OBJ)

# Compilation du serveur
$(SERVER_BIN): $(SERVER_SRC) $(OBJ)
	$(CC) $(SERVER_SRC) $(OBJ) -o $(SERVER_BIN)

# Compilation du client
$(CLIENT_BIN): $(CLIENT_SRC) $(OBJ)
	$(CC) $(CLIENT_SRC) $(OBJ) -o $(CLIENT_BIN)

# Nettoyage des fichiers générés
clean:
	rm -f $(OBJ) $(SERVER_BIN) $(CLIENT_BIN)
