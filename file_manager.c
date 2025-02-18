#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "file_manager.h"

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
void handle_command() {
    char input[100];

    while (1) {
        printf("\n请输入命令 (create/list/copy/move/delete/chmod/exit): ");
        fgets(input, sizeof(input), stdin);

        // 去掉换行符
        input[strcspn(input, "\n")] = '\0';

        // 退出命令
        if (strcmp(input, "exit") == 0) {
            printf("sorti,bey!\n");
            break;
        }

        // 按空格拆分输入
        char *argv[10];
        int argc = 0;
        char *token = strtok(input, " ");
        while (token != NULL && argc < 10) {
            argv[argc++] = token;
            token = strtok(NULL, " ");
        }

        // 解析命令
        if (argc == 0) continue;  // 如果没有输入内容，继续等待输入

        const char *command = argv[0];

        if (strcmp(command, "create") == 0 && argc == 4) {
            create_file(argv[1], atoi(argv[2]), atoi(argv[3]));
        } else if (strcmp(command, "list") == 0) {
            list_files();
        } else if (strcmp(command, "copy") == 0 && argc == 3) {
            copy_file(argv[1], argv[2]);
        } else if (strcmp(command, "move") == 0 && argc == 3) {
            move_file(argv[1], argv[2]);
        } else if (strcmp(command, "delete") == 0 && argc == 2) {
            delete_file(argv[1]);
        } else if (strcmp(command, "chmod") == 0 && argc == 3) {
            set_permissions(argv[1], atoi(argv[2]));
        } else {
            printf("Commande non reconnue ou arguments invalides.\n");
        }
    }
}
