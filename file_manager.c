#include <stdio.h>
#include <string.h>
#include <stdlib.h>
// fcntl.h - Pour les opérations sur les fichiers (open, O_RDWR)
#include <fcntl.h>     
// unistd.h - Pour les opérations système (read, write, close)
#include <unistd.h>
// sys/stat.h - Pour les permissions des fichiers
#include <sys/stat.h>
// ctype.h - Pour le traitement des caractères (isspace)
#include <ctype.h>
// file_manager.h - Définitions des structures et constantes du système de fichiers
#include "file_manager.h"

FileNode* root_directory = NULL;
FileNode* current_directory = NULL;
int fs_fd;

// Initialiser le système de fichiers
void init_file_system() {
    fs_fd = open(FS_FILENAME, O_RDWR | O_CREAT, 0644);
    if (fs_fd < 0) {
        perror("Erreur lors de l'ouverture du fichier système");
        exit(EXIT_FAILURE);
    }

    // Essayer de charger le système de fichiers existant
    if (load_file_system() != 0) {
    // Si le chargement échoue, créer un nouveau système de fichiers
    root_directory = (FileNode*)malloc(sizeof(FileNode));
    strcpy(root_directory->name, "/");
    root_directory->type = DIRECTORY_TYPE;
    root_directory->permissions = 755;
    root_directory->size = 0;
    root_directory->parent = NULL;
    root_directory->child_count = 0;
    root_directory->content = NULL;
    root_directory->is_open = 0;
    root_directory->ref_count = 1;
    root_directory->symlink_target = NULL;
    }
    current_directory = root_directory;
}

// Sauvegarder un répertoire et ses contenus récursivement
void save_directory(int fd, FileNode* dir) {
    if (!dir) return;
    
    // écrire le noeud
    write(fd, dir, sizeof(FileNode));
    
    // récursively save children
    for (int i = 0; i < dir->child_count; i++) {
        save_directory(fd, dir->children[i]);
    }
}

// Sauvegarder le système de fichiers
void save_file_system() {
    if (fs_fd < 0) return;
    
    // Tronquer le fichier à zéro octet
    ftruncate(fs_fd, 0);
    lseek(fs_fd, 0, SEEK_SET);
    
    // Sauvegarder le système de fichiers
    save_directory(fs_fd, root_directory);
}

// Récursivement charger un répertoire
FileNode* load_directory(int fd) {
    FileNode* node = (FileNode*)malloc(sizeof(FileNode));
    
    // Lire le noeud
    if (read(fd, node, sizeof(FileNode)) <= 0) {
        free(node);
        return NULL;
    }
    
    // Récursivement charger les enfants
    for (int i = 0; i < node->child_count; i++) {
        node->children[i] = load_directory(fd);
        if (node->children[i]) {
            node->children[i]->parent = node;
        }
    }
    
    return node;
}

// Charger le système de fichiers
int load_file_system() {
    if (fs_fd < 0) return -1;
    
    lseek(fs_fd, 0, SEEK_SET);
    root_directory = load_directory(fs_fd);
    
    return root_directory ? 0 : -1;
}

// Fermer le système de fichiers
void close_file_system() {
    if (fs_fd >= 0) {
        save_file_system();
        close(fs_fd);
        fs_fd = -1;
    }
}

// Analyser le chemin
FileNode* get_file_by_path(const char* path) {
    // Traiter les cas spéciaux pour la racine et le répertoire courant
    if (strcmp(path, "/") == 0) return root_directory;
    if (strcmp(path, ".") == 0) return current_directory;
    
    // Copier le chemin pour l'analyse
    char path_copy[MAX_PATH_LENGTH];
    strncpy(path_copy, path, MAX_PATH_LENGTH - 1);
    path_copy[MAX_PATH_LENGTH - 1] = '\0';
    
    // Déterminer le répertoire de départ
    FileNode* current = (path_copy[0] == '/') ? root_directory : current_directory;
    if (path_copy[0] == '/') {
        memmove(path_copy, path_copy + 1, strlen(path_copy));
    }
    
    // Diviser le chemin par '/' et rechercher niveau par niveau
    char* token = strtok(path_copy, "/");
    while (token != NULL) {
        if (strcmp(token, ".") == 0) {
            // Current directory
        } else if (strcmp(token, "..") == 0) {
            // Parent directory
            if (current->parent != NULL) {
                current = current->parent;
            }
        } else {
            // Chercher le fichier dans les enfants
            int found = 0;
            for (int i = 0; i < current->child_count; i++) {
                if (strcmp(current->children[i]->name, token) == 0) {
                    current = current->children[i];
                    found = 1;
                    break;
                }
            }
            if (!found) {
                // Si c'est le dernier composant du chemin, retourner le répertoire parent
                // pour permettre la création de nouveaux fichiers/répertoires
                if (strtok(NULL, "/") == NULL) {
                    return current;
                }
                // Si ce n'est pas le dernier composant, le chemin est invalide
                return NULL;
            }
        }
        token = strtok(NULL, "/");
    }
    
    return current;
}

// Créer un fichier
int create_file(const char* path, int permissions, int size) {
    // Obtenir le répertoire parent et le nom du fichier
    char path_copy[MAX_PATH_LENGTH];
    char *filename;
    strncpy(path_copy, path, MAX_PATH_LENGTH - 1);
    
    // Extraire le nom du fichier à partir du chemin complet
    filename = strrchr(path_copy, '/');
    filename = filename ? filename + 1 : path_copy;
    
    FileNode* parent = get_file_by_path(path);
    if (parent == NULL || parent->type != DIRECTORY_TYPE) {
        printf("Erreur : chemin invalide.\n");
        return -1;
    }
    
    if (parent->child_count >= MAX_FILES) {
        printf("Erreur : répertoire plein.\n");
        return -1;
    }

    FileNode* new_file = (FileNode*)malloc(sizeof(FileNode));
    strncpy(new_file->name, filename, MAX_NAME_LENGTH - 1);
    //Initialiser le fichier
    new_file->name[MAX_NAME_LENGTH - 1] = '\0';
    new_file->type = FILE_TYPE;
    new_file->permissions = permissions;
    new_file->size = size;
    new_file->parent = parent;
    new_file->child_count = 0;
    new_file->content = NULL;         
    new_file->is_open = 0;  
    new_file->ref_count = 1;        
    new_file->symlink_target = NULL; 

    parent->children[parent->child_count++] = new_file;
    printf("Fichier '%s' créé avec permissions %d et taille %d.\n", path, permissions, size);
    return 0;
}

// Créer un répertoire
int create_directory(const char* path, int permissions) {
    // Obtenir le répertoire parent et le nom du répertoire
    char path_copy[MAX_PATH_LENGTH];
    char *dirname;
    strncpy(path_copy, path, MAX_PATH_LENGTH - 1);
    
    // Extraire le nom du répertoire à partir du chemin complet
    dirname = strrchr(path_copy, '/');
    dirname = dirname ? dirname + 1 : path_copy;
    
    FileNode* parent = get_file_by_path(path);
    if (parent == NULL || parent->type != DIRECTORY_TYPE) {
        printf("Erreur : chemin invalide.\n");
        return -1;
    }
    
    if (parent->child_count >= MAX_FILES) {
        printf("Erreur : répertoire plein.\n");
        return -1;
    }

    FileNode* new_dir = (FileNode*)malloc(sizeof(FileNode));
    strncpy(new_dir->name, dirname, MAX_NAME_LENGTH - 1);  // Utiliser le nom extrait
    new_dir->name[MAX_NAME_LENGTH - 1] = '\0';
    new_dir->type = DIRECTORY_TYPE;
    new_dir->permissions = permissions;
    new_dir->size = 0;
    new_dir->parent = parent;
    new_dir->child_count = 0;

    parent->children[parent->child_count++] = new_dir;
    printf("Répertoire '%s' créé avec permissions %d.\n", path, permissions);
    return 0;
}

// Lister les fichiers
void list_files(const char* path) {
    char path_copy[MAX_PATH_LENGTH];
    strncpy(path_copy, path, MAX_PATH_LENGTH - 1);
    path_copy[MAX_PATH_LENGTH - 1] = '\0';
    
    FileNode* dir = get_file_by_path(path);
    if (dir == NULL || dir->type != DIRECTORY_TYPE) {
        printf("Erreur : chemin invalide.\n");
        return;
    }
    
    if (dir->child_count == 0) {
        printf("Répertoire vide.\n");
        return;
    }

    printf("Contenu du répertoire '%s' :\n", path);
    for (int i = 0; i < dir->child_count; i++) {
        FileNode* node = dir->children[i];
        printf("%s %s, permissions : %d", 
            node->type == DIRECTORY_TYPE ? "Répertoire" : "Fichier",
            node->name, 
            node->permissions);
        if (node->type == FILE_TYPE) {
            printf(", taille : %d", node->size);
        }
        printf("\n");
    }
}

// Changer de répertoire
int change_directory(const char* path) {
    if (strcmp(path, "..") == 0) {
        // Retourner au répertoire parent
        if (current_directory->parent != NULL) {
            current_directory = current_directory->parent;
            printf("Changement vers le répertoire parent\n");
            return 0;
        } else {
            printf("Erreur : déjà à la racine\n");
            return -1;
        }
    } else if (strcmp(path, "/") == 0) {
        // Aller au répertoire racine
        current_directory = root_directory;
        printf("Changement vers le répertoire racine\n");
        return 0;
    } else if (strcmp(path, ".") == 0) {
        // Current directory, faire rien
        return 0;
    } else {
        FileNode* target = NULL;
        
        // Trouver le répertoire cible
        if (path[0] != '/') {
            // Change to a subdirectory
            for (int i = 0; i < current_directory->child_count; i++) {
                if (strcmp(current_directory->children[i]->name, path) == 0 && 
                    current_directory->children[i]->type == DIRECTORY_TYPE) {
                    target = current_directory->children[i];
                    break;
                }
            }
        } else {
            // Trouver le répertoire cible à partir de la racine
            target = get_file_by_path(path);
            if (target != NULL && target->type != DIRECTORY_TYPE) {
                target = NULL;
            }
        }
        
        if (target == NULL) {
            printf("Erreur : répertoire '%s' non trouvé\n", path);
            return -1;
        }
        
        current_directory = target;
        printf("Changement vers le répertoire '%s'\n", path);
        return 0;
    }
}

// Copier un fichier
int copy_file(const char* source, const char* destination) {
    // Obtenir les informations du chemin source
    char src_path[MAX_PATH_LENGTH];
    char *src_name;
    strncpy(src_path, source, MAX_PATH_LENGTH - 1);
    src_path[MAX_PATH_LENGTH - 1] = '\0';
    
    // Charger le fichier source
    char parent_path[MAX_PATH_LENGTH] = ".";
    src_name = strrchr(src_path, '/');
    if (src_name) {
        strncpy(parent_path, src_path, src_name - src_path);
        parent_path[src_name - src_path] = '\0';
        src_name++;
    } else {
        src_name = src_path;
    }
    
    FileNode* parent = get_file_by_path(parent_path);
    if (parent == NULL || parent->type != DIRECTORY_TYPE) {
        printf("Erreur : chemin source invalide.\n");
        return -1;
    }
    
    // CHarger le fichier source
    FileNode* src_file = NULL;
    for (int i = 0; i < parent->child_count; i++) {
        if (strcmp(parent->children[i]->name, src_name) == 0 && 
            parent->children[i]->type == FILE_TYPE) {
            src_file = parent->children[i];
            break;
        }
    }
    
    if (src_file == NULL) {
        printf("Erreur : fichier source '%s' non trouvé.\n", src_name);
        return -1;
    }
    
    return create_file(destination, src_file->permissions, src_file->size);
}

int move_file(const char* source, const char* destination) {
    // Obtenir les informations du chemin source
    char src_path[MAX_PATH_LENGTH];
    char *src_name;
    strncpy(src_path, source, MAX_PATH_LENGTH - 1);
    src_path[MAX_PATH_LENGTH - 1] = '\0';
    
    // Chercher le fichier source dans le répertoire parent
    char parent_path[MAX_PATH_LENGTH] = ".";
    src_name = strrchr(src_path, '/');
    if (src_name) {
        strncpy(parent_path, src_path, src_name - src_path);
        parent_path[src_name - src_path] = '\0';
        src_name++;
    } else {
        src_name = src_path;
    }
    
    FileNode* parent = get_file_by_path(parent_path);
    if (parent == NULL || parent->type != DIRECTORY_TYPE) {
        printf("Erreur : chemin source invalide.\n");
        return -1;
    }
    
    // Chercher le fichier source dans le répertoire parent
    FileNode* src_file = NULL;
    int src_index = -1;
    for (int i = 0; i < parent->child_count; i++) {
        if (strcmp(parent->children[i]->name, src_name) == 0) {
            src_file = parent->children[i];
            src_index = i;
            break;
        }
    }
    
    if (src_file == NULL) {
        printf("Erreur : fichier source '%s' non trouvé.\n", src_name);
        return -1;
    }
    
    if (create_file(destination, src_file->permissions, src_file->size) == 0) {
        // Supprimer le fichier source
        for (int i = src_index; i < parent->child_count - 1; i++) {
            parent->children[i] = parent->children[i + 1];
        }
        parent->child_count--;
        free(src_file);
        return 0;
    }
    return -1;
}

// Supprimer un fichier
// Supprimer récursivement un répertoire et son contenu
void recursive_delete(FileNode* node) {
    // Supprimer d'abord tous les nœuds enfants récursivement
    if (node == NULL) return;
    
    while (node->child_count > 0) {
        recursive_delete(node->children[node->child_count - 1]);
        node->child_count--;
    }
    
    
    free(node);
}

int delete_file(const char* filename) {
    // Obtenir le nom du fichier après le dernier '/'
    char path_copy[MAX_PATH_LENGTH];
    char *name;
    strncpy(path_copy, filename, MAX_PATH_LENGTH - 1);
    path_copy[MAX_PATH_LENGTH - 1] = '\0';
    
    // Extraire le nom du fichier après le dernier '/'
    name = strrchr(path_copy, '/');
    name = name ? name + 1 : path_copy;
    
    // Extraction du nom du fichier à partir du chemin complet
    char parent_path[MAX_PATH_LENGTH] = ".";
    if (name != path_copy) {
        strncpy(parent_path, path_copy, name - path_copy);
        parent_path[name - path_copy - 1] = '\0';
    }
    
    FileNode* parent = get_file_by_path(parent_path);
    if (parent == NULL || parent->type != DIRECTORY_TYPE) {
        printf("Erreur : chemin invalide.\n");
        return -1;
    }
    
    // Trouver le fichier cible
    FileNode* target = NULL;
    int target_index = -1;
    for (int i = 0; i < parent->child_count; i++) {
        if (strcmp(parent->children[i]->name, name) == 0) {
            target = parent->children[i];
            target_index = i;
            break;
        }
    }
    
    if (target == NULL) {
        printf("Erreur : fichier '%s' non trouvé.\n", name);
        return -1;
    }
    
    // Si c'est un répertoire, supprimer récursivement
    if (target->type == DIRECTORY_TYPE) {
        recursive_delete(target);
    } else {
        free(target);
    }
    
    // Supprimer le nœud du parent
    for (int i = target_index; i < parent->child_count - 1; i++) {
        parent->children[i] = parent->children[i + 1];
    }
    parent->child_count--;
    
    printf("%s '%s' supprimé.\n", 
           target->type == DIRECTORY_TYPE ? "Répertoire" : "Fichier", 
           name);
    return 0;
}

// Modifier les permissions
int set_permissions(const char* filename, int permissions) {
    // Obtenir le nom du fichier après le dernier '/'
    char path_copy[MAX_PATH_LENGTH];
    char *name;
    strncpy(path_copy, filename, MAX_PATH_LENGTH - 1);
    path_copy[MAX_PATH_LENGTH - 1] = '\0';
    
    
    name = strrchr(path_copy, '/');
    name = name ? name + 1 : path_copy;
    
    
    char parent_path[MAX_PATH_LENGTH] = ".";
    if (name != path_copy) {
        strncpy(parent_path, path_copy, name - path_copy);
        parent_path[name - path_copy - 1] = '\0';
    }
    
    FileNode* parent = get_file_by_path(parent_path);
    if (parent == NULL || parent->type != DIRECTORY_TYPE) {
        printf("Erreur : chemin invalide.\n");
        return -1;
    }
    
    
    FileNode* target = NULL;
    for (int i = 0; i < parent->child_count; i++) {
        if (strcmp(parent->children[i]->name, name) == 0) {
            target = parent->children[i];
            break;
        }
    }
    
    if (target == NULL) {
        printf("Erreur : fichier '%s' non trouvé.\n", name);
        return -1;
    }
    
    target->permissions = permissions;
    printf("Permissions du fichier '%s' modifiées à %d.\n", name, permissions);
    return 0;
}

// Ouvrir un fichier
int open_file(const char* path, const char* mode) {
    FileNode* file = get_file_by_path(path);
    if (file == NULL || file->type != FILE_TYPE) {
        printf("Erreur : fichier '%s' non trouvé.\n", path);
        return -1;
    }

    int owner_perm = (file->permissions / 100);
    int requested_mode = 0;

    if (strcmp(mode, "r") == 0) {
        if (!(owner_perm & 4)) {
            printf("Erreur : permission de lecture refusée.\n");
            return -1;
        }
        requested_mode = FILE_MODE_READ;
    } else if (strcmp(mode, "w") == 0) {
        if (!(owner_perm & 2)) {
            printf("Erreur : permission d'écriture refusée.\n");
            return -1;
        }
        requested_mode = FILE_MODE_WRITE;
    } else if (strcmp(mode, "rw") == 0) {
        if (!(owner_perm & 6)) {
            printf("Erreur : permissions insuffisantes.\n");
            return -1;
        }
        requested_mode = FILE_MODE_BOTH;
    } else {
        printf("Erreur : mode d'ouverture invalide.\n");
        return -1;
    }

    if (file->is_open) {
        printf("Erreur : fichier déjà ouvert.\n");
        return -1;
    }

    file->is_open = 1;
    file->open_mode = requested_mode;
    printf("Fichier '%s' ouvert en mode %s.\n", path, mode);
    return 0;
}

int read_file(const char* path, char* buffer, int size) {
    FileNode* file = get_file_by_path(path);
    if (file == NULL || file->type != FILE_TYPE) {
        printf("Erreur : fichier '%s' non trouvé.\n", path);
        return -1;
    }

    if (!file->is_open) {
        printf("Erreur : fichier non ouvert.\n");
        return -1;
    }

    if (!(file->open_mode & FILE_MODE_READ)) {
        printf("Erreur : fichier non ouvert en lecture.\n");
        return -1;
    }

    if (file->content == NULL) {
        printf("Fichier vide.\n");
        buffer[0] = '\0';
        return 0;
    }

    strncpy(buffer, file->content, size);
    buffer[size - 1] = '\0';
    printf("Contenu lu : %s\n", buffer);
    return strlen(buffer);
}

int write_file(const char* path, const char* content) {
    FileNode* file = get_file_by_path(path);
    if (file == NULL || file->type != FILE_TYPE) {
        printf("Erreur : fichier '%s' non trouvé.\n", path);
        return -1;
    }

    if (!file->is_open) {
        printf("Erreur : fichier non ouvert.\n");
        return -1;
    }

    if (!(file->open_mode & FILE_MODE_WRITE)) {
        printf("Erreur : fichier non ouvert en écriture.\n");
        return -1;
    }

    if (file->content != NULL) {
        free(file->content);
    }

    file->content = strdup(content);
    file->size = strlen(content);
    printf("Contenu écrit dans '%s'.\n", path);
    return 0;
}

int close_file(const char* path) {
    FileNode* file = get_file_by_path(path);
    if (file == NULL || file->type != FILE_TYPE) {
        printf("Erreur : fichier '%s' non trouvé.\n", path);
        return -1;
    }

    if (!file->is_open) {
        printf("Erreur : fichier non ouvert.\n");
        return -1;
    }

    file->is_open = 0;
    file->open_mode = 0;  // 清除打开模式
    printf("Fichier '%s' fermé.\n", path);
    return 0;
}

// Créer un lien dur
int create_hard_link(const char* target, const char* link_name) {
    FileNode* target_file = get_file_by_path(target);
    if (target_file == NULL || target_file->type != FILE_TYPE) {
        printf("Erreur : fichier cible '%s' non trouvé.\n", target);
        return -1;
    }

    target_file->ref_count++;
    FileNode* link = (FileNode*)malloc(sizeof(FileNode));
    memcpy(link, target_file, sizeof(FileNode));
    strncpy(link->name, link_name, MAX_NAME_LENGTH - 1);
    link->name[MAX_NAME_LENGTH - 1] = '\0';

    FileNode* parent = get_file_by_path(".");
    parent->children[parent->child_count++] = link;
    printf("Lien dur '%s' créé vers '%s'.\n", link_name, target);
    return 0;
}

// Créer un lien symbolique
int create_symbolic_link(const char* target, const char* link_name) {
    FileNode* link = (FileNode*)malloc(sizeof(FileNode));
    strncpy(link->name, link_name, MAX_NAME_LENGTH - 1);
    link->name[MAX_NAME_LENGTH - 1] = '\0';
    link->type = FILE_TYPE;
    link->permissions = 777;
    link->size = 0;
    link->symlink_target = strdup(target);
    link->is_open = 0;
    link->ref_count = 1;

    FileNode* parent = get_file_by_path(".");
    parent->children[parent->child_count++] = link;
    printf("Lien symbolique '%s' créé vers '%s'.\n", link_name, target);
    return 0;
}

// Traiter les commandes
void handle_command() {
    // Vérifier si le répertoire racine existe
    if (root_directory == NULL) {
        init_file_system();
    }

    char input[1024];
    while (1) {
        printf("\nEntrez une commande (create/mkdir/ls/copy/move/rm/chmod/cd/open/close/read/write/ln/exit) : ");
        fgets(input, sizeof(input), stdin);
        input[strcspn(input, "\n")] = '\0';

        if (strcmp(input, "exit") == 0) {
            printf("Sauvegarde du système de fichiers...\n");
            close_file_system();
            printf("Au revoir !\n");
            break;
        }

        char *argv[10];
        int argc = 0;
        char *token = NULL;
        char *rest = input;
        char *quote_start = NULL;

        while (*rest) {
            // Passer les espaces blancs
            while (*rest && isspace(*rest)) rest++;
            if (!*rest) break;

            if (*rest == '"') {
                // Traiter les chaînes entre guillemets
                rest++; 
                quote_start = rest;
                while (*rest && *rest != '"') rest++;
                if (*rest == '"') {
                    *rest = '\0'; 
                    argv[argc++] = quote_start;
                    rest++;
                } else {
                    // Non fermere les guillemets
                    argv[argc++] = quote_start;
                    break;
                }
            } else {
                // Traiter les mots séparés par des espaces
                token = rest;
                while (*rest && !isspace(*rest) && *rest != '"') rest++;
                if (*rest) {
                    *rest++ = '\0';
                }
                argv[argc++] = token;
            }

            if (argc >= 10) break;
        }

        if (argc == 0) continue;

        const char *command = argv[0];

        if (strcmp(command, "create") == 0 && argc == 4) {
            create_file(argv[1], atoi(argv[2]), atoi(argv[3]));
        } else if (strcmp(command, "mkdir") == 0 && argc == 3) {
            create_directory(argv[1], atoi(argv[2]));
        } else if (strcmp(command, "ls") == 0 || strcmp(command, "list") == 0) {
            list_files(argc > 1 ? argv[1] : ".");
        } else if (strcmp(command, "cd") == 0 && argc == 2) {
            change_directory(argv[1]);
        } else if (strcmp(command, "copy") == 0 && argc == 3) {
            copy_file(argv[1], argv[2]);
        } else if (strcmp(command, "move") == 0 && argc == 3) {
            move_file(argv[1], argv[2]);
        } else if ((strcmp(command, "rm") == 0 || strcmp(command, "delete") == 0) && argc >= 2) {
            // Verifier si la commande est rm -r
            if (argc == 3 && strcmp(argv[1], "-r") == 0) {
                // rm -r 
                delete_file(argv[2]);
            } else {
                // Vrifier si le chemin est un répertoire
                char path_copy[MAX_PATH_LENGTH];
                strncpy(path_copy, argv[1], MAX_PATH_LENGTH - 1);
                path_copy[MAX_PATH_LENGTH - 1] = '\0';
                
                FileNode* target = get_file_by_path(path_copy);
                if (target != NULL && target->type == DIRECTORY_TYPE) {
                    printf("Erreur : '%s' est un répertoire. Utilisez 'rm -r' pour supprimer un répertoire.\n", argv[1]);
                } else {
                    delete_file(argv[1]);
                }
            }
        } else if (strcmp(command, "chmod") == 0 && argc == 3) {
            set_permissions(argv[1], atoi(argv[2]));
        } else if (strcmp(command, "open") == 0 && argc == 3) {
            open_file(argv[1], argv[2]);
        } else if (strcmp(command, "close") == 0 && argc == 2) {
            close_file(argv[1]);
        } else if (strcmp(command, "read") == 0 && argc == 2) {
            char buffer[1024];
            read_file(argv[1], buffer, sizeof(buffer));
        } else if (strcmp(command, "write") == 0 && argc == 3) {
            write_file(argv[1], argv[2]);
        } else if (strcmp(command, "ln") == 0 && argc == 3) {
            create_hard_link(argv[1], argv[2]);
        } else if (strcmp(command, "ln") == 0 && argc == 4 && strcmp(argv[1], "-s") == 0) {
            create_symbolic_link(argv[2], argv[3]);
        }else {
            printf("Commande non reconnue ou arguments invalides.\n");
            printf("Usage:\n");
            printf("  create <fichier> <permissions> <taille>\n");
            printf("  mkdir <répertoire> <permissions>\n");
            printf("  ls [chemin]\n");
            printf("  cd <chemin>\n");
            printf("  copy <source> <destination>\n");
            printf("  move <source> <destination>\n");
            printf("  rm [-r] <chemin>\n");
            printf("  chmod <chemin> <permissions>\n");
            printf("  open <fichier> <mode>     (mode: r ou w)\n");
            printf("  close <fichier>\n");
            printf("  read <fichier>\n");
            printf("  write <fichier> <contenu>\n");
            printf("  ln <source> <lien>        (lien dur)\n");
            printf("  ln -s <source> <lien>     (lien symbolique)\n");
            printf("  exit\n");
        }
    }
}
