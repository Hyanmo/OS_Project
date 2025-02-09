#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "file_manager.h"

// 模拟文件系统
#define MAX_FILES 100
typedef struct {
    char name[50];
    int permissions;
    int size;
} File;

File file_system[MAX_FILES];
int file_count = 0;

// 创建文件
int create_file(const char *filename, int permissions, int size) {
    if (file_count >= MAX_FILES) {
        printf("Erreur: limite de fichiers atteinte.\n");
        return -1;
    }
    strcpy(file_system[file_count].name, filename);
    file_system[file_count].permissions = permissions;
    file_system[file_count].size = size;
    file_count++;

    printf("Fichier '%s' créé avec les permissions %d et la taille %d.\n", filename, permissions, size);
    return 0;
}

// 列出文件
void list_files() {
    if (file_count == 0) {
        printf("Aucun fichier trouvé.\n");
        return;
    }

    printf("Liste des fichiers :\n");
    for (int i = 0; i < file_count; i++) {
        printf("Nom: %s, Permissions: %d, Taille: %d\n", file_system[i].name, file_system[i].permissions, file_system[i].size);
    }
}

// 复制文件
int copy_file(const char *source, const char *destination) {
    for (int i = 0; i < file_count; i++) {
        if (strcmp(file_system[i].name, source) == 0) {
            return create_file(destination, file_system[i].permissions, file_system[i].size);
        }
    }
    printf("Erreur: fichier '%s' non trouvé.\n", source);
    return -1;
}

// 移动文件
int move_file(const char *source, const char *destination) {
    for (int i = 0; i < file_count; i++) {
        if (strcmp(file_system[i].name, source) == 0) {
            if (create_file(destination, file_system[i].permissions, file_system[i].size) == 0) {
                delete_file(source);
                return 0;
            }
        }
    }
    printf("Erreur: fichier '%s' non trouvé.\n", source);
    return -1;
}

// 删除文件
int delete_file(const char *filename) {
    for (int i = 0; i < file_count; i++) {
        if (strcmp(file_system[i].name, filename) == 0) {
            for (int j = i; j < file_count - 1; j++) {
                file_system[j] = file_system[j + 1];
            }
            file_count--;
            printf("Fichier '%s' supprimé.\n", filename);
            return 0;
        }
    }
    printf("Erreur: fichier '%s' non trouvé.\n", filename);
    return -1;
}

// 修改文件权限
int set_permissions(const char *filename, int permissions) {
    for (int i = 0; i < file_count; i++) {
        if (strcmp(file_system[i].name, filename) == 0) {
            file_system[i].permissions = permissions;
            printf("Permissions de '%s' modifiées en %d.\n", filename, permissions);
            return 0;
        }
    }
    printf("Erreur: fichier '%s' non trouvé.\n", filename);
    return -1;
}

// 处理命令
void handle_command(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: ./file_manager <command> [arguments]\n");
        return;
    }

    const char *command = argv[1];

    if (strcmp(command, "create") == 0 && argc == 5) {
        // 创建文件
        const char *filename = argv[2];
        int permissions = atoi(argv[3]);
        int size = atoi(argv[4]);
        create_file(filename, permissions, size);
    } else if (strcmp(command, "list") == 0) {
        // 列出文件
        list_files();
    } else if (strcmp(command, "copy") == 0 && argc == 4) {
        // 复制文件
        const char *source = argv[2];
        const char *destination = argv[3];
        copy_file(source, destination);
    } else if (strcmp(command, "move") == 0 && argc == 4) {
        // 移动文件
        const char *source = argv[2];
        const char *destination = argv[3];
        move_file(source, destination);
    } else if (strcmp(command, "delete") == 0 && argc == 3) {
        // 删除文件
        const char *filename = argv[2];
        delete_file(filename);
    } else if (strcmp(command, "chmod") == 0 && argc == 4) {
        // 修改文件权限
        const char *filename = argv[2];
        int permissions = atoi(argv[3]);
        set_permissions(filename, permissions);
    } else {
        printf("Commande non reconnue ou arguments invalides.\n");
    }
}
