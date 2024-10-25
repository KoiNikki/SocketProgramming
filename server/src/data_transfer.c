#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include "data_transfer.h"

// 用于存储客户端指定的地址和端口
struct sockaddr_in client_data_addr;
int passive_socket = -1;

// 打开数据连接
int open_data_connection() {
    if (passive_socket != -1) {
        int data_socket = accept(passive_socket, NULL, NULL);
        if (data_socket < 0) {
            perror("Accept failed");
            close(passive_socket);
            passive_socket = -1;
            return -1;
        }
        close(passive_socket);
        passive_socket = -1;
        return data_socket;
    } else if (client_data_addr.sin_port != 0) {
        int data_socket = socket(AF_INET, SOCK_STREAM, 0);
        if (data_socket < 0) {
            perror("Socket creation failed");
            return -1;
        }
        if (connect(data_socket, (struct sockaddr *)&client_data_addr, sizeof(client_data_addr)) < 0) {
            perror("Connect failed");
            close(data_socket);
            return -1;
        }
        return data_socket;
    } else {
        return -1;
    }
}


void handle_port_command(int client_socket, const char *buffer) {
    int h1, h2, h3, h4, p1, p2;
    char ip[16];
    int port;

    // 解析 PORT 命令中的 IP 地址和端口
    if (sscanf(buffer, "PORT %d,%d,%d,%d,%d,%d", &h1, &h2, &h3, &h4, &p1, &p2) != 6) {
        send(client_socket, "500 Syntax error, command unrecognized.\r\n", 41, 0);
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
    if (inet_pton(AF_INET, ip, &client_data_addr.sin_addr) <= 0) {
        send(client_socket, "500 Invalid IP address.\r\n", 25, 0);
        return;
    }

    // 发送成功响应
    send(client_socket, "200 PORT command successful.\r\n", 30, 0);
}


void handle_pasv_command(int client_socket) {
    // 随机生成一个端口（20000-65535范围内）
    int port = 20000 + rand() % 45535;

    // 创建新的被动模式套接字
    passive_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (passive_socket < 0) {
        perror("Passive socket creation failed");
        send(client_socket, "500 Failed to enter passive mode.\r\n", 35, 0);
        return;
    }

    struct sockaddr_in pasv_addr;
    memset(&pasv_addr, 0, sizeof(pasv_addr));
    pasv_addr.sin_family = AF_INET;
    pasv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    pasv_addr.sin_port = htons(port);

    if (bind(passive_socket, (struct sockaddr *)&pasv_addr, sizeof(pasv_addr)) < 0) {
        perror("Bind failed");
        close(passive_socket);
        send(client_socket, "500 Bind failed.\r\n", 18, 0);
        return;
    }

    if (listen(passive_socket, 1) < 0) {
        perror("Listen failed");
        close(passive_socket);
        send(client_socket, "500 Listen failed.\r\n", 20, 0);
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

    send(client_socket, response, strlen(response), 0);
}

// 实现 LIST 命令
void handle_list_command(int client_socket, const char *directory) {
    int data_socket = open_data_connection();
    if (data_socket < 0) {
        send(client_socket, "425 Can't open data connection.\r\n", 33, 0);
        return;
    }

    send(client_socket, "150 Here comes the directory listing.\r\n", 39, 0);

    DIR *dir = opendir(directory);
    struct dirent *entry;
    char buffer[1024];

    if (dir == NULL) {
        send(client_socket, "550 Failed to open directory.\r\n", 31, 0);
        close(data_socket);
        return;
    }

    while ((entry = readdir(dir)) != NULL) {
        snprintf(buffer, sizeof(buffer), "%s\r\n", entry->d_name);
        send(data_socket, buffer, strlen(buffer), 0);
    }

    closedir(dir);
    close(data_socket);
    send(client_socket, "226 Directory send OK.\r\n", 24, 0);
}

// 实现 RETR 命令
void handle_retr_command(int client_socket, const char *filename) {
    int data_socket = open_data_connection();
    if (data_socket < 0) {
        send(client_socket, "425 Can't open data connection.\r\n", 33, 0);
        return;
    }

    int file_fd = open(filename, O_RDONLY);
    if (file_fd < 0) {
        send(client_socket, "550 Failed to open file.\r\n", 26, 0);
        close(data_socket);
        return;
    }

    send(client_socket, "150 Opening binary mode data connection.\r\n", 42, 0);

    char buffer[1024];
    int bytes_read;
    while ((bytes_read = read(file_fd, buffer, sizeof(buffer))) > 0) {
        send(data_socket, buffer, bytes_read, 0);
    }

    close(file_fd);
    close(data_socket);
    send(client_socket, "226 Transfer complete.\r\n", 24, 0);
}

// 实现 STOR 命令
void handle_stor_command(int client_socket, const char *filename) {
    int data_socket = open_data_connection();
    if (data_socket < 0) {
        send(client_socket, "425 Can't open data connection.\r\n", 33, 0);
        return;
    }

    int file_fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0666);
    if (file_fd < 0) {
        send(client_socket, "550 Failed to create file.\r\n", 28, 0);
        close(data_socket);
        return;
    }

    send(client_socket, "150 Opening binary mode data connection.\r\n", 42, 0);

    char buffer[1024];
    int bytes_received;
    while ((bytes_received = recv(data_socket, buffer, sizeof(buffer), 0)) > 0) {
        write(file_fd, buffer, bytes_received);
    }

    close(file_fd);
    close(data_socket);
    send(client_socket, "226 Transfer complete.\r\n", 24, 0);
}