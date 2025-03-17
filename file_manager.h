#ifndef FILE_MANAGER_H
#define FILE_MANAGER_H

// Simulation du système de fichiers
#define MAX_FILES 100
#define MAX_PATH_LENGTH 256
#define MAX_NAME_LENGTH 50

typedef enum {
    FILE_TYPE,
    DIRECTORY_TYPE
} FileType;

typedef struct FileNode {
    char name[MAX_NAME_LENGTH];
    FileType type;
    int permissions;
    int size;
    struct FileNode* parent;
    struct FileNode* children[MAX_FILES];
    int child_count;
} FileNode;

extern FileNode* root_directory;  // Répertoire racine
extern FileNode* current_directory;  // Répertoire courant

// Fonctions d'opération du système de fichiers
int create_file(const char* path, int permissions, int size);
int create_directory(const char* path, int permissions);
void list_files(const char* path);
int copy_file(const char* source, const char* destination);
int move_file(const char* source, const char* destination);
int delete_file(const char* path);
int set_permissions(const char* path, int permissions);
int change_directory(const char* path);
FileNode* get_file_by_path(const char* path);

// Fonction pour gérer les commandes de l'utilisateur
void handle_command();

#endif // FILE_MANAGER_H
