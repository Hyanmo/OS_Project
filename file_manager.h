#ifndef FILE_MANAGER_H
#define FILE_MANAGER_H

// Structure représentant un fichier
typedef struct {
    char name[256];  // Nom du fichier
    int size;        // Taille du fichier
    int permissions; // Permissions du fichier (lecture, écriture, exécution)
    int start_block; // Bloc de départ du fichier dans le système
} File;

// Création d'un fichier
int create_file(const char *name, int permissions, int size);

// Suppression d'un fichier
int delete_file(const char *name);

// Copie d'un fichier
int copy_file(const char *src_name, const char *dst_name);

// Déplacement d'un fichier
int move_file(const char *src_name, const char *dst_name);

// Modification des permissions
int set_permissions(const char *name, int permissions);

// Liste des fichiers
void list_files();

#endif
