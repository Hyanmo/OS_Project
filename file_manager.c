#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "file_manager.h"

FileNode* root_directory = NULL;
FileNode* current_directory = NULL;

// Initialiser le système de fichiers
void init_file_system() {
    root_directory = (FileNode*)malloc(sizeof(FileNode));
    strcpy(root_directory->name, "/");
    root_directory->type = DIRECTORY_TYPE;
    root_directory->permissions = 755;
    root_directory->size = 0;
    root_directory->parent = NULL;
    root_directory->child_count = 0;
    current_directory = root_directory;
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
            // 当前目录，不需要操作
        } else if (strcmp(token, "..") == 0) {
            // 父目录
            if (current->parent != NULL) {
                current = current->parent;
            }
        } else {
            // 查找子目录或文件
            int found = 0;
            for (int i = 0; i < current->child_count; i++) {
                if (strcmp(current->children[i]->name, token) == 0) {
                    current = current->children[i];
                    found = 1;
                    break;
                }
            }
            if (!found) {
                // 如果是最后一个组件，可能是新文件名，返回父目录
                if (strtok(NULL, "/") == NULL) {
                    return current;
                }
                return NULL;  // 路径无效
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
    
    // 获取最后一个'/'后的文件名
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
    strncpy(new_file->name, filename, MAX_NAME_LENGTH - 1);  // 使用提取的文件名
    new_file->name[MAX_NAME_LENGTH - 1] = '\0';
    new_file->type = FILE_TYPE;
    new_file->permissions = permissions;
    new_file->size = size;
    new_file->parent = parent;
    new_file->child_count = 0;

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
    
    // 获取最后一个'/'后的目录名
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
    strncpy(new_dir->name, dirname, MAX_NAME_LENGTH - 1);  // 使用提取的目录名
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
        // 当前目录，不需要操作
        return 0;
    } else {
        FileNode* target = NULL;
        
        // 处理相对路径
        if (path[0] != '/') {
            // 在当前目录下查找
            for (int i = 0; i < current_directory->child_count; i++) {
                if (strcmp(current_directory->children[i]->name, path) == 0 && 
                    current_directory->children[i]->type == DIRECTORY_TYPE) {
                    target = current_directory->children[i];
                    break;
                }
            }
        } else {
            // 处理绝对路径
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
    
    // 获取源文件的父目录路径
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
    
    // 在父目录中查找源文件
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
    
    // 获取源文件的父目录路径
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
    
    // 在父目录中查找源文件
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
        // 从源目录中删除文件
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
    
    // 获取最后一个'/'后的文件名
    name = strrchr(path_copy, '/');
    name = name ? name + 1 : path_copy;
    
    // 获取父目录路径
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
    
    // 在父目录中查找目标文件
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
    
    // 如果是目录，递归删除
    if (target->type == DIRECTORY_TYPE) {
        recursive_delete(target);
    } else {
        free(target);
    }
    
    // 从父目录中移除该节点
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

// Traiter les commandes
void handle_command() {
    // Vérifier si le répertoire racine existe
    if (root_directory == NULL) {
        init_file_system();
    }

    char input[100];
    while (1) {
        printf("\nEntrez une commande (create/mkdir/ls/copy/move/rm/chmod/cd/exit) : ");
        fgets(input, sizeof(input), stdin);
        input[strcspn(input, "\n")] = '\0';

        if (strcmp(input, "exit") == 0) {
            printf("Au revoir !\n");
            break;
        }

        char *argv[10];
        int argc = 0;
        char *token = strtok(input, " ");
        while (token != NULL && argc < 10) {
            argv[argc++] = token;
            token = strtok(NULL, " ");
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
            // 检查是否为递归删除
            if (argc == 3 && strcmp(argv[1], "-r") == 0) {
                // rm -r 命令
                delete_file(argv[2]);
            } else {
                // 检查目标是否为目录
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
        } else {
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
            printf("  exit\n");
        }
    }
}
