/**
 * @file file_manager.c
 * @brief Implémentation d'un système de fichiers virtuel
 *
 * Ce fichier contient l'implémentation d'un système de fichiers simple en mémoire.
 * Il fournit les fonctionnalités suivantes:
 * - Gestion des fichiers et répertoires
 * - Gestion des permissions
 * - Liens durs et symboliques
 * - Persistance des données
 *
 * @author BAKHOUCHE Rachel|HUANG Yanmo|ANAGONOU Hervé
 * @date 2024
 */

/**
 * @brief Inclusions des bibliothèques nécessaires
 */
#include <stdio.h>      /**< Pour les opérations d'entrée/sortie */
#include <string.h>     /**< Pour la manipulation des chaînes */
#include <stdlib.h>     /**< Pour les fonctions malloc, free */
#include <fcntl.h>      /**< Pour les opérations sur les fichiers (open, O_RDWR) */
#include <unistd.h>     /**< Pour les opérations système (read, write, close) */
#include <sys/stat.h>   /**< Pour les permissions des fichiers */
#include <ctype.h>      /**< Pour le traitement des caractères (isspace) */
#include "file_manager.h" /**< Définitions des structures et constantes */

/**
 * @brief Variables globales du système de fichiers
 */

/** @brief Répertoire racine du système de fichiers */
FileNode* root_directory = NULL;

/** @brief Répertoire de travail actuel */
FileNode* current_directory = NULL;

/** @brief Descripteur de fichier pour le stockage persistant */
int fs_fd;

/**
 * @brief Initialise le système de fichiers
 *
 * Cette fonction crée ou charge le système de fichiers.
 * Si un système existant est trouvé, il est chargé.
 * Sinon, un nouveau système est créé avec un répertoire racine.
 */
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

/**
 * @brief Sauvegarde récursivement un répertoire et son contenu
 * 
 * Cette fonction sauvegarde un nœud de répertoire et tous ses enfants
 * dans le fichier de stockage persistant.
 * 
 * @param fd Descripteur du fichier de stockage
 * @param dir Pointeur vers le nœud de répertoire à sauvegarder
 * 
 * @details
 * - Vérifie d'abord si le nœud est valide
 * - Écrit le nœud courant dans le fichier
 * - Sauvegarde récursivement tous les enfants
 */
void save_directory(int fd, FileNode* dir) {
    if (!dir) return;
    
    // écrire le noeud
    write(fd, dir, sizeof(FileNode));
    
    // récursively save children
    for (int i = 0; i < dir->child_count; i++) {
        save_directory(fd, dir->children[i]);
    }
}

/**
 * @brief Sauvegarde l'état complet du système de fichiers
 * 
 * Cette fonction sauvegarde l'intégralité du système de fichiers
 * dans le fichier de stockage persistant.
 * 
 * @details
 * - Vérifie si le descripteur de fichier est valide
 * - Tronque le fichier existant
 * - Réinitialise le pointeur de fichier
 * - Lance la sauvegarde récursive depuis la racine
 */
void save_file_system() {
    if (fs_fd < 0) return;
    
    // Tronquer le fichier à zéro octet
    ftruncate(fs_fd, 0);
    lseek(fs_fd, 0, SEEK_SET);
    
    // Sauvegarder le système de fichiers
    save_directory(fs_fd, root_directory);
}

/**
 * @brief Charge récursivement un répertoire depuis le stockage
 * 
 * @param fd Descripteur du fichier de stockage
 * @return FileNode* Pointeur vers le nœud chargé, NULL en cas d'erreur
 * 
 * @details
 * - Alloue la mémoire pour un nouveau nœud
 * - Lit les données du nœud depuis le fichier
 * - Charge récursivement tous les enfants
 * - Établit les liens parent-enfant
 */
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

/**
 * @brief Charge le système de fichiers depuis le stockage
 * 
 * @return int 0 en cas de succès, -1 en cas d'échec
 * 
 * @details
 * - Vérifie si le descripteur de fichier est valide
 * - Réinitialise le pointeur de fichier
 * - Lance le chargement récursif depuis la racine
 */
int load_file_system() {
    if (fs_fd < 0) return -1;
    
    lseek(fs_fd, 0, SEEK_SET);
    root_directory = load_directory(fs_fd);
    
    return root_directory ? 0 : -1;
}

// Fermer le système de fichiers
/**
 * @brief Ferme proprement le système de fichiers
 * 
 * Cette fonction effectue la fermeture propre du système de fichiers,
 * en sauvegardant l'état actuel et en libérant les ressources.
 * 
 * @details
 * - Vérifie si le descripteur de fichier est valide
 * - Sauvegarde l'état actuel du système de fichiers
 * - Ferme le fichier de stockage
 * - Réinitialise le descripteur de fichier
 */
void close_file_system() {
    if (fs_fd >= 0) {
        save_file_system();
        close(fs_fd);
        fs_fd = -1;
    }
}

/**
 * @brief Analyse un chemin et retourne le nœud correspondant
 * 
 * @param path Le chemin à analyser
 * @return FileNode* Pointeur vers le nœud trouvé, NULL si le chemin est invalide
 * 
 * @details
 * - Gère les chemins absolus (commençant par '/') et relatifs
 * - Traite les cas spéciaux '.' (répertoire courant) et '..' (répertoire parent)
 * - Pour un nouveau fichier/répertoire, retourne le parent si le chemin n'existe pas
 */
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


/**
 * @brief Crée un nouveau fichier dans le système
 * 
 * @param path Chemin du fichier à créer
 * @param permissions Permissions du fichier (format octal)
 * @return int 0 en cas de succès, -1 en cas d'erreur
 * 
 * @details
 * - Vérifie si le répertoire parent existe et est valide
 * - Vérifie si le répertoire n'est pas plein
 * - Initialise un nouveau nœud de type fichier
 * - Met à jour la structure du répertoire parent
 */
int create_file(const char* path, int permissions) {
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
    new_file->name[MAX_NAME_LENGTH - 1] = '\0';
    new_file->type = FILE_TYPE;
    new_file->permissions = permissions;
    new_file->size = 0;  // initialize size 0
    new_file->parent = parent;
    new_file->child_count = 0;
    new_file->content = NULL;         
    new_file->is_open = 0;  
    new_file->ref_count = 1;        
    new_file->symlink_target = NULL; 

    parent->children[parent->child_count++] = new_file;
    printf("Fichier '%s' créé avec permissions %d.\n", path, permissions);
    return 0;
}


/**
 * @brief Crée un nouveau répertoire dans le système
 * 
 * @param path Chemin du répertoire à créer
 * @param permissions Permissions du répertoire (format octal)
 * @return int 0 en cas de succès, -1 en cas d'erreur
 * 
 * @details
 * - Vérifie si le répertoire parent existe et est valide
 * - Vérifie si le répertoire parent n'est pas plein
 * - Initialise un nouveau nœud de type répertoire
 * - Met à jour la structure du répertoire parent
 */
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

/**
 * @brief Liste le contenu d'un répertoire
 * 
 * @param path Chemin du répertoire à lister
 * 
 * @details
 * - Affiche le chemin complet du répertoire
 * - Pour chaque entrée, affiche :
 *   - Le type (fichier ou répertoire)
 *   - Le nom
 *   - Les permissions
 *   - La taille (pour les fichiers uniquement)
 */
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

    // Afficher le chemin complet du répertoire
    printf("Contenu du répertoire '%s' :\n", 
           strcmp(path, ".") == 0 ? get_current_path() : path);

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

/**
 * @brief Change le répertoire de travail courant
 * 
 * @param path Chemin du répertoire cible
 * @return int 0 en cas de succès, -1 en cas d'erreur
 * 
 * @details
 * - Gère les cas spéciaux : ".." (parent), "/" (racine), "." (courant)
 * - Vérifie si le chemin cible est un répertoire valide
 * - Met à jour le répertoire de travail courant
 */
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

/**
 * @brief Copie un fichier vers une nouvelle destination
 * 
 * @param source Chemin du fichier source
 * @param destination Chemin de destination
 * @return int 0 en cas de succès, -1 en cas d'erreur
 * 
 * @details
 * - Vérifie l'existence et la validité du fichier source
 * - Crée un nouveau fichier à la destination
 * - Copie les permissions et le contenu
 * - Gère les erreurs de chemin et de permissions
 */
int copy_file(const char* source, const char* destination) {
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
    
    // Charger le fichier source
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
    
    // Copyer le fichier source
    if (create_file(destination, src_file->permissions) == 0) {
        // Copyer le contenu du fichier source
        FileNode* dest_file = get_file_by_path(destination);
        if (dest_file != NULL && src_file->content != NULL) {
            dest_file->content = strdup(src_file->content);
            dest_file->size = src_file->size;
            printf("Fichier '%s' copié vers '%s'.\n", source, destination);
            return 0;
        }
    }
    return -1;
}

/**
 * @brief Déplace un fichier vers une nouvelle destination
 * 
 * @param source Chemin du fichier source
 * @param destination Chemin de destination
 * @return int 0 en cas de succès, -1 en cas d'erreur
 * 
 * @details
 * - Vérifie l'existence et la validité du fichier source
 * - Crée un nouveau fichier à la destination
 * - Copie les permissions et le contenu
 * - Supprime le fichier source après la copie
 */
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
    
    if (create_file(destination, src_file->permissions) == 0) {
        // Copyer le contenu du fichier source
        FileNode* dest_file = get_file_by_path(destination);
        if (dest_file != NULL && src_file->content != NULL) {
            dest_file->content = strdup(src_file->content);
            dest_file->size = src_file->size;
            
            // Supprimer le fichier source
            for (int i = src_index; i < parent->child_count - 1; i++) {
                parent->children[i] = parent->children[i + 1];
            }
            parent->child_count--;
            free(src_file);
            printf("Fichier '%s' déplacé vers '%s'.\n", source, destination);
            return 0;
        }
    }
    return -1;
}

/**
 * @brief Supprime récursivement un nœud et tous ses enfants
 * 
 * @param node Pointeur vers le nœud à supprimer
 * 
 * @details
 * - Supprime récursivement tous les nœuds enfants
 * - Libère la mémoire allouée pour le nœud
 * - Gère les cas de répertoires et fichiers
 */
void recursive_delete(FileNode* node) {
    // Supprimer d'abord tous les nœuds enfants récursivement
    if (node == NULL) return;
    
    while (node->child_count > 0) {
        recursive_delete(node->children[node->child_count - 1]);
        node->child_count--;
    }
    
    
    free(node);
}

/**
 * @brief Supprime un fichier ou un répertoire
 * 
 * @param filename Chemin du fichier ou répertoire à supprimer
 * @return int 0 en cas de succès, -1 en cas d'erreur
 * 
 * @details
 * - Vérifie l'existence et la validité du chemin
 * - Gère la suppression récursive pour les répertoires
 * - Met à jour la structure du répertoire parent
 */
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

/**
 * @brief Modifie les permissions d'un fichier ou répertoire
 * 
 * @param filename Chemin du fichier ou répertoire
 * @param permissions Nouvelles permissions (format octal)
 * @return int 0 en cas de succès, -1 en cas d'erreur
 * 
 * @details
 * - Vérifie l'existence et la validité du chemin
 * - Met à jour les permissions du fichier ou répertoire
 * - Gère les erreurs de chemin invalide
 */
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

/**
 * @brief Ouvre un fichier en mode lecture ou écriture
 * 
 * @param path Chemin du fichier à ouvrir
 * @param mode Mode d'ouverture ("r" pour lecture, "w" pour écriture, "rw" pour les deux)
 * @return int 0 en cas de succès, -1 en cas d'erreur
 * 
 * @details
 * - Vérifie l'existence et le type du fichier
 * - Vérifie les permissions d'accès
 * - Gère les différents modes d'ouverture
 * - Empêche l'ouverture multiple d'un même fichier
 */
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

/**
 * @brief Lit le contenu d'un fichier
 * 
 * @param path Chemin du fichier à lire
 * @param buffer Buffer pour stocker le contenu lu
 * @param size Taille maximale du buffer
 * @return int Nombre d'octets lus, -1 en cas d'erreur
 * 
 * @details
 * - Vérifie si le fichier est ouvert en lecture
 * - Copie le contenu dans le buffer fourni
 * - Gère la taille maximale du buffer
 * - Ajoute le caractère nul à la fin
 */
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

    // Assurer que le buffer est suffisamment grand
    int copy_size = (size - 1 < file->size) ? (size - 1) : file->size;
    strncpy(buffer, file->content, copy_size);
    buffer[copy_size] = '\0';
    printf("Contenu lu : %s\n", buffer);
    return copy_size;
}

/**
 * @brief Écrit du contenu dans un fichier
 * 
 * @param path Chemin du fichier
 * @param content Contenu à écrire
 * @return int Nombre d'octets écrits, -1 en cas d'erreur
 * 
 * @details
 * - Vérifie si le fichier est ouvert en écriture
 * - Libère l'ancien contenu si nécessaire
 * - Alloue de la mémoire pour le nouveau contenu
 * - Met à jour la taille du fichier
 */
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

    // Libre la mémoire actuelle du contenu du fichier
    if (file->content != NULL) {
        free(file->content);
    }

    // Alloue de la mémoire pour le nouveau contenu
    file->content = strdup(content);
    file->size = strlen(content);
    printf("Contenu écrit dans '%s' (taille: %d octets).\n", path, file->size);
    return file->size;
}

/**
 * @brief Ferme un fichier ouvert
 * 
 * @param path Chemin du fichier à fermer
 * @return int 0 en cas de succès, -1 en cas d'erreur
 * 
 * @details
 * - Vérifie si le fichier existe et est ouvert
 * - Réinitialise les flags d'ouverture
 * - Libère les ressources associées
 */
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
    file->open_mode = 0;  // Réinitialiser le mode d'ouverture
    printf("Fichier '%s' fermé.\n", path);
    return 0;
}

/**
 * @brief Obtient le chemin absolu du répertoire de travail actuel
 * 
 * @return char* Chaîne de caractères représentant le chemin absolu
 * 
 * @details
 * - Utilise un buffer statique pour stocker le chemin
 * - Remonte l'arborescence depuis le répertoire courant jusqu'à la racine
 * - Gère le cas spécial du répertoire racine "/"
 * - Construit le chemin en concaténant les noms des répertoires
 */
char* get_current_path() {
    static char path[MAX_PATH_LENGTH];
    FileNode* current = current_directory;
    path[0] = '\0';
    
    // Si c'est le répertoire racine, retourner "/"
    if (current == root_directory) {
        return "/";
    }
    
    // Construire le chemin complet
    while (current != root_directory) {
        char temp[MAX_PATH_LENGTH];
        snprintf(temp, sizeof(temp), "/%s%s", current->name, path);
        strncpy(path, temp, MAX_PATH_LENGTH - 1);
        current = current->parent;
    }
    
    return path[0] ? path : "/";
}

/**
 * @brief Crée un lien dur vers un fichier existant
 * 
 * @param target Chemin du fichier cible
 * @param link_name Nom du nouveau lien dur
 * @return int 0 en cas de succès, -1 en cas d'erreur
 * 
 * @details
 * - Vérifie l'existence et le type du fichier cible
 * - Incrémente le compteur de références du fichier
 * - Crée une nouvelle entrée dans le répertoire courant
 * - Partage le même contenu que le fichier cible
 */
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

/**
 * @brief Crée un lien symbolique vers un fichier ou répertoire
 * 
 * @param target Chemin de la cible
 * @param link_name Nom du nouveau lien symbolique
 * @return int 0 en cas de succès, -1 en cas d'erreur
 * 
 * @details
 * - Crée un nouveau nœud de type lien symbolique
 * - Stocke le chemin de la cible
 * - Ne vérifie pas l'existence de la cible (lien symbolique peut être cassé)
 * - Initialise les attributs du lien
 */
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

/**
 * @brief Traite les commandes utilisateur du système de fichiers
 * 
 * @details
 * - Initialise le système de fichiers si nécessaire
 * - Boucle principale de lecture des commandes
 * - Parse les arguments en gérant les guillemets
 * - Exécute la commande correspondante
 * - Gère les erreurs et affiche l'aide si nécessaire
 * - Sauvegarde l'état à la sortie
 */
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

        if (strcmp(command, "create") == 0 && argc == 3) {  
            create_file(argv[1], atoi(argv[2]));  
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
            printf("  create <fichier> <permissions>\n");
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
