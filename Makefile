# Définition du compilateur
CC = gcc
# Options de compilation : -Wall pour les avertissements, -g pour le débogage
CFLAGS = -Wall -g
# Nom du programme final
TARGET = file_manager
# Liste des fichiers objets nécessaires
OBJ = file_manager.o main.o

# Cible par défaut
all: $(TARGET)

# Création de l'exécutable
$(TARGET): $(OBJ)
	$(CC) $(OBJ) -o $(TARGET)

# Compilation de file_manager.c
file_manager.o: file_manager.c file_manager.h
	$(CC) $(CFLAGS) -c file_manager.c

# Compilation de main.c
main.o: main.c file_manager.h
	$(CC) $(CFLAGS) -c main.c

# Nettoyage des fichiers générés
clean:
	rm -f $(OBJ) $(TARGET)
