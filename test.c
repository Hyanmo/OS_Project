#include <stdio.h>
#include <string.h>

#define MAX_FILES 100

typedef struct {
    char name[50];
    int permissions;
    int size;
} File;

extern File file_system[MAX_FILES];
extern int file_count = 0;

void create_file(const char *name, int permissions, int size) {
    if (file_count >= MAX_FILES) {
        printf("Error: File system is full.\n");
        return;
    }

    strcpy(file_system[file_count].name, name);
    file_system[file_count].permissions = permissions;
    file_system[file_count].size = size;
    file_count++;  // 增加文件计数
}

void list_files() {
    if (file_count == 0) {
        printf("No files in the system.\n");
        return;
    }

    printf("File List:\n");
    for (int i = 0; i < file_count; i++) {
        printf("Name: %s, Permissions: %d, Size: %d bytes\n",
               file_system[i].name, file_system[i].permissions, file_system[i].size);
    }
}

int main() {
    create_file("file1.txt", 644, 1024);
    create_file("file2.txt", 600, 2048);

    list_files();  // 显示所有文件
    return 0;
}
