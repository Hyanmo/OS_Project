#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "file_manager.h"

// Simuler un système de fichiers avec un tableau de fichiers
#define MAX_FILES 100
File file_system[MAX_FILES];  // Tableau de fichiers
int file_count = 0;           // Compteur de fichiers dans le système

int create_file(const char *name, int permissions, int size) {
    if (file_count >= MAX_FILES) {
        printf("Le système de fichiers est plein !\n");
        return -1;
    }
    
    // Créer un fichier
    File new_file;
    strncpy(new_file.name, name, sizeof(new_file.name) - 1);
    new_file.permissions = permissions;
    new_file.size = size;
    new_file.start_block = file_count;  // Bloc de départ du fichier

    file_system[file_count++] = new_file;  // Ajouter le fichier au système
    printf("Le fichier '%s' a été créé avec succès !\n", name);
    return 0;
}

int delete_file(const char *name) {
    for (int i = 0; i < file_count; ++i) {
        if (strcmp(file_system[i].name, name) == 0) {
            // Supprimer le fichier (en décalant les éléments)
            for (int j = i; j < file_count - 1; ++j) {
                file_system[j] = file_system[j + 1];
            }
            --file_count;  // Réduire le nombre de fichiers
            printf("Le fichier '%s' a été supprimé.\n", name);
            return 0;
        }
    }
    printf("Le fichier '%s' n'a pas été trouvé.\n", name);
    return -1;
}

int copy_file(const char *src_name, const char *dst_name) {
    for (int i = 0; i < file_count; ++i) {
        if (strcmp(file_system[i].name, src_name) == 0) {
            // Copier le fichier en créant un nouveau fichier
            return create_file(dst_name, file_system[i].permissions, file_system[i].size);
        }
    }
    printf("Le fichier '%s' n'a pas été trouvé.\n", src_name);
    return -1;
}

int move_file(const char *src_name, const char *dst_name) {
    for (int i = 0; i < file_count; ++i) {
        if (strcmp(file_system[i].name, src_name) == 0) {
            // Déplacer le fichier (en supprimant l'ancien et en créant un nouveau)
            delete_file(src_name);
            return create_file(dst_name, file_system[i].permissions, file_system[i].size);
        }
    }
    printf("Le fichier '%s' n'a pas été trouvé.\n", src_name);
    return -1;
}

int set_permissions(const char *name, int permissions) {
    for (int i = 0; i < file_count; ++i) {
        if (strcmp(file_system[i].name, name) == 0) {
            file_system[i].permissions = permissions;
            printf("Les permissions du fichier '%s' ont été mises à jour.\n", name);
            return 0;
        }
    }
    printf("Le fichier '%s' n'a pas été trouvé.\n", name);
    return -1;
}

void list_files() {
    if (file_count == 0) {
        printf("Aucun fichier dans le système.\n");
        return;
    }
    
    printf("Fichiers dans le système :\n");
    for (int i = 0; i < file_count; ++i) {
        printf("Nom : %s, Taille : %d, Permissions : %d\n", file_system[i].name, file_system[i].size, file_system[i].permissions);
    }
}
