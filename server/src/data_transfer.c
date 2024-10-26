#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <linux/limits.h>
#include <sys/stat.h>
#include "data_transfer.h"

// 用于存储客户端指定的地址和端口
struct sockaddr_in client_data_addr;
int passive_socket = -1;

// 打开数据连接
int open_data_connection()
{
    if (passive_socket != -1)
    {
        int data_socket = accept(passive_socket, NULL, NULL);
        if (data_socket < 0)
        {
            perror("Accept failed");
            close(passive_socket);
            passive_socket = -1;
            return -1;
        }
        close(passive_socket);
        passive_socket = -1;
        return data_socket;
    }
    else if (client_data_addr.sin_port != 0)
    {
        int data_socket = socket(AF_INET, SOCK_STREAM, 0);
        if (data_socket < 0)
        {
            perror("Socket creation failed");
            return -1;
        }
        if (connect(data_socket, (struct sockaddr *)&client_data_addr, sizeof(client_data_addr)) < 0)
        {
            perror("Connect failed");
            close(data_socket);
            return -1;
        }
        return data_socket;
    }
    else
    {
        return -1;
    }
}

void handle_port_command(int client_socket, const char *buffer)
{
    int h1, h2, h3, h4, p1, p2;
    char ip[16];
    int port;

    // 解析 PORT 命令中的 IP 地址和端口
    if (sscanf(buffer, "PORT %d,%d,%d,%d,%d,%d", &h1, &h2, &h3, &h4, &p1, &p2) != 6)
    {
        send_response(client_socket, "500 Syntax error, command unrecognized.\r\n");
        return;
    }

    // 构造 IP 地址字符串
    snprintf(ip, sizeof(ip), "%d.%d.%d.%d", h1, h2, h3, h4);

    // 计算端口号
    port = p1 * 256 + p2;

    // 清零 client_data_addr 结构体
    memset(&client_data_addr, 0, sizeof(client_data_addr));

    // 设置客户端数据地址结构
    client_data_addr.sin_family = AF_INET;
    client_data_addr.sin_port = htons(port);
    if (inet_pton(AF_INET, ip, &client_data_addr.sin_addr) <= 0)
    {
        send_response(client_socket, "500 Invalid IP address.\r\n");
        return;
    }

    // 发送成功响应
    send_response(client_socket, "200 PORT command successful.\r\n");
}

void handle_pasv_command(int client_socket)
{
    // 随机生成一个端口（20000-65535范围内）
    int port = 20000 + rand() % 45535;

    // 创建新的被动模式套接字
    passive_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (passive_socket < 0)
    {
        perror("Passive socket creation failed");
        send_response(client_socket, "500 Failed to enter passive mode.\r\n");
        return;
    }

    struct sockaddr_in pasv_addr;
    memset(&pasv_addr, 0, sizeof(pasv_addr));
    pasv_addr.sin_family = AF_INET;
    pasv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    pasv_addr.sin_port = htons(port);

    if (bind(passive_socket, (struct sockaddr *)&pasv_addr, sizeof(pasv_addr)) < 0)
    {
        perror("Bind failed");
        close(passive_socket);
        send_response(client_socket, "500 Bind failed.\r\n");
        return;
    }

    if (listen(passive_socket, 1) < 0)
    {
        perror("Listen failed");
        close(passive_socket);
        send_response(client_socket, "500 Listen failed.\r\n");
        return;
    }

    // 获取服务器的IP地址
    struct sockaddr_in server_addr;
    socklen_t addr_len = sizeof(server_addr);
    getsockname(client_socket, (struct sockaddr *)&server_addr, &addr_len);

    // 将IP地址和端口转换为FTP格式
    unsigned char *ip = (unsigned char *)&server_addr.sin_addr.s_addr;
    unsigned char *p = (unsigned char *)&pasv_addr.sin_port;
    char response[50];
    snprintf(response, sizeof(response), "227 Entering Passive Mode (%d,%d,%d,%d,%d,%d).\r\n",
             ip[0], ip[1], ip[2], ip[3], p[0], p[1]);

    send_response(client_socket, response);
}

// 实现 LIST 命令
void handle_list_command(int client_socket, char *current_directory)
{
    int data_socket = open_data_connection();
    if (data_socket < 0)
    {
        send_response(client_socket, "425 Can't open data connection.\r\n");
        return;
    }

    send_response(client_socket, "150 Here comes the directory listing.\r\n");

    DIR *dir = opendir(current_directory);
    struct dirent *entry;
    char buffer[1024];

    if (dir == NULL)
    {
        send_response(client_socket, "550 Failed to open directory.\r\n");
        close(data_socket);
        return;
    }

    while ((entry = readdir(dir)) != NULL)
    {
        snprintf(buffer, sizeof(buffer), "%s\r\n", entry->d_name);
        send(data_socket, buffer, strlen(buffer), 0);
    }

    closedir(dir);
    close(data_socket);
    send_response(client_socket, "226 Directory send OK.\r\n");
}

// 实现 RETR 命令
void handle_retr_command(int client_socket, const char *filename)
{
    int data_socket = open_data_connection();
    if (data_socket < 0)
    {
        send_response(client_socket, "425 Can't open data connection.\r\n");
        return;
    }

    int file_fd = open(filename, O_RDONLY);
    if (file_fd < 0)
    {
        send_response(client_socket, "550 Failed to open file.\r\n");
        close(data_socket);
        return;
    }

    send_response(client_socket, "150 Opening binary mode data connection.\r\n");

    char buffer[1024];
    int bytes_read;
    while ((bytes_read = read(file_fd, buffer, sizeof(buffer))) > 0)
    {
        send(data_socket, buffer, bytes_read, 0);
    }

    close(file_fd);
    close(data_socket);
    send_response(client_socket, "226 Transfer complete.\r\n");
}

// 实现 STOR 命令
void handle_stor_command(int client_socket, const char *filename)
{
    int data_socket = open_data_connection();
    if (data_socket < 0)
    {
        send_response(client_socket, "226 Transfer complete.\r\n");
        return;
    }

    int file_fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0666);
    if (file_fd < 0)
    {
        send_response(client_socket, "550 Failed to create file.\r\n");
        close(data_socket);
        return;
    }

    send_response(client_socket, "150 Opening binary mode data connection.\r\n");

    char buffer[1024];
    int bytes_received;
    while ((bytes_received = recv(data_socket, buffer, sizeof(buffer), 0)) > 0)
    {
        write(file_fd, buffer, bytes_received);
    }

    close(file_fd);
    close(data_socket);
    send_response(client_socket, "226 Transfer complete.\r\n");
}

// 辅助函数：生成相对路径
void get_relative_path(const char *root_directory, const char *current_directory, char *relative_directory)
{
    if (strncmp(current_directory, root_directory, strlen(root_directory)) == 0)
    {
        snprintf(relative_directory, PATH_MAX, "%s", current_directory + strlen(root_directory));
        if (relative_directory[0] == '\0')
        {
            // 如果没有相对路径部分，表示在根目录，显示为 "/"
            strcpy(relative_directory, "/");
        }
    }
    else
    {
        // 如果出错，返回根目录
        strcpy(relative_directory, "/");
    }
}

void handle_pwd_command(int client_socket, const char *current_directory, const char *root_directory)
{
    char response[512];
    char relative_directory[PATH_MAX];

    // 获取相对于根目录的路径
    get_relative_path(root_directory, current_directory, relative_directory);

    // 返回相对路径
    snprintf(response, sizeof(response), "257 \"%.400s\" is the current directory.\r\n", relative_directory);
    send(client_socket, response, strlen(response), 0);
}

// 辅助函数：检查新路径是否在根目录范围内
int is_within_root(const char *root_directory, const char *new_directory)
{
    char real_root[PATH_MAX], real_new[PATH_MAX];

    // 获取 root_directory 和 new_directory 的真实绝对路径
    realpath(root_directory, real_root);
    realpath(new_directory, real_new);

    // 检查 new_directory 是否以 root_directory 为前缀
    return strncmp(real_root, real_new, strlen(real_root)) == 0;
}

void handle_cwd_command(int client_socket, const char *buffer, char *current_directory, const char *root_directory)
{
    char new_path[PATH_MAX];
    char target_directory[PATH_MAX];

    // 从 CWD 命令中提取目标路径
    sscanf(buffer, "CWD %s", target_directory);

    // 检查路径长度是否会超出 PATH_MAX
    size_t current_len = strlen(current_directory);
    size_t target_len = strlen(target_directory);

    if (current_len + 1 + target_len + 1 > PATH_MAX) // +1 for '/' and +1 for null terminator
    {
        send_response(client_socket, "550 Path too long.\r\n");
        return;
    }

    // 初始化 new_path 并拼接路径
    strncpy(new_path, current_directory, PATH_MAX - 1);  // 复制 current_directory
    new_path[PATH_MAX - 1] = '\0';  // 确保以 NULL 结尾

    if (current_len > 0 && new_path[current_len - 1] != '/') {
        strncat(new_path, "/", PATH_MAX - strlen(new_path) - 1);  // 添加斜杠
    }

    strncat(new_path, target_directory, PATH_MAX - strlen(new_path) - 1);  // 拼接目标路径

    // 检查是否超出根目录范围
    if (!is_within_root(root_directory, new_path))
    {
        send_response(client_socket, "550 Access denied. Cannot move outside root directory.\r\n");
        return;
    }

    // 尝试切换到新路径
    if (chdir(new_path) == 0)
    {
        // 使用 realpath 更新 current_directory 为 new_path 的真实绝对路径
        // char* q = realpath(new_path, current_directory);
        if (realpath(new_path, current_directory) != NULL)
        {
            send_response(client_socket, "250 Directory successfully changed.\r\n");
        }
        else
        {
            send_response(client_socket, "550 Failed to resolve new directory path.\r\n");
        }
    }
    else
    {
        send_response(client_socket, "550 Failed to change directory. Directory does not exist.\r\n");
    }
}

void handle_mkd_command(int client_socket, const char *dirpath)
{
    if (mkdir(dirpath, 0777) == 0)
    {
        send_response(client_socket, "257 Directory created successfully.\r\n");
    }
    else
    {
        send_response(client_socket, "550 Failed to create directory.\r\n");
    }
}

void handle_rmd_command(int client_socket, const char *dirpath)
{
    if (rmdir(dirpath) == 0)
    {
        send_response(client_socket, "250 Directory removed.\r\n");
    }
    else
    {
        send_response(client_socket, "550 Failed to remove directory.\r\n");
    }
}

void send_response(int client_socket, const char *response) {
    size_t len = strlen(response);
    send(client_socket, response, len, 0);
}