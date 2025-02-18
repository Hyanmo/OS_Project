#ifndef FILE_MANAGER_H
#define FILE_MANAGER_H

// 模拟文件系统
#define MAX_FILES 100
typedef struct {
    char name[50];
    int permissions;
    int size;
} File;

extern File file_system[MAX_FILES];  // 让其他文件可以访问 file_system
extern int file_count;               // 让其他文件可以访问 file_cou

// 创建文件
int create_file(const char *filename, int permissions, int size);

// 列出文件
void list_files();

// 复制文件
int copy_file(const char *source, const char *destination);

// 移动文件
int move_file(const char *source, const char *destination);

// 删除文件
int delete_file(const char *filename);

// 修改文件权限
int set_permissions(const char *filename, int permissions);

// 处理命令
void handle_command();

#endif // FILE_MANAGER_H
