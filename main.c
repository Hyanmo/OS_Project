#include <stdio.h>
#include "file_manager.h"

int main() {
    // Création de fichiers
    create_file("fichier1.txt", 755, 1024);
    create_file("fichier2.txt", 644, 512);

    // Liste des fichiers
    list_files();

    // Copier un fichier
    copy_file("fichier1.txt", "fichier3.txt");

    // Liste après la copie
    list_files();

    // Déplacer un fichier
    move_file("fichier2.txt", "fichier4.txt");

    // Liste après le déplacement
    list_files();

    // Supprimer un fichier
    delete_file("fichier3.txt");

    // Liste après la suppression
    list_files();

    // Modifier les permissions d'un fichier
    set_permissions("fichier1.txt", 777);

    return 0;
}
