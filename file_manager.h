#ifndef FILE_MANAGER_H
#define FILE_MANAGER_H

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
void handle_command(int argc, char *argv[]);

#endif // FILE_MANAGER_H
