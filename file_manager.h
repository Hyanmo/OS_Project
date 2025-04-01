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

// Définition des modes d'ouverture de fichier
#define FILE_MODE_READ  1
#define FILE_MODE_WRITE 2
#define FILE_MODE_BOTH  3
#define FS_FILENAME "filesystem.dat"

typedef struct FileNode {
    char name[MAX_NAME_LENGTH]; // Nom du fichier ou du répertoire
    FileType type; // Type de fichier (fichier ou répertoire)
    int permissions; // Permissions du fichier ou du répertoire
    int size; // Taille du fichier en octets
    struct FileNode* parent; // Répertoire parent
    struct FileNode* children[MAX_FILES]; // Sous-répertoires ou fichiers
    int child_count; // Nombre de sous-répertoires ou fichiers
    char* content;           // Contenu du fichier
    int is_open;            // État du fichier (ouvert/fermé)
    int ref_count;          // Nombre de références (pour les liens durs)
    char* symlink_target;   // Cible du lien symbolique
    int open_mode;          // Mode d'ouverture du fichier
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
int open_file(const char* path, const char* mode);    // mode: "r" ou "w"
int close_file(const char* path);
int read_file(const char* path, char* buffer, int size);
int write_file(const char* path, const char* content);
int create_hard_link(const char* target, const char* link_name);
int create_symbolic_link(const char* target, const char* link_name);

// Fonctions pour la gestion du système de fichiers
void init_file_system();
void save_file_system();
void close_file_system();
int load_file_system();

// Fonction pour gérer les commandes de l'utilisateur
void handle_command();

// Obtenir le chemin complet du répertoire courant
char* get_current_path();

#endif // FILE_MANAGER_H
