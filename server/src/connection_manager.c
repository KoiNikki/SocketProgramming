#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "connection_manager.h"
#include "auth.h"

// 发送欢迎消息
void handle_welcome(int client_socket) {
    send(client_socket, "220 Anonymous FTP server ready.\r\n", 34, 0);
}

// 处理 USER 命令
void handle_user_command(int client_socket, const char *buffer) {
    char username[100];
    sscanf(buffer, "USER %s", username);
    if (validate_user(username)) {
        send(client_socket, "331 Please specify the password.\r\n", 34, 0);
    } else {
        send(client_socket, "530 Invalid username.\r\n", 23, 0);
    }
}

// 处理 PASS 命令
int handle_pass_command(int client_socket, const char *buffer) {
    char password[100];
    sscanf(buffer, "PASS %s", password);
    if (validate_password(password)) {
        send(client_socket, "230 Login successful.\r\n", 23, 0);
        return 1;  // 登录成功
    } else {
        send(client_socket, "530 Invalid password.\r\n", 23, 0);
        return 0;
    }
}

// 处理客户端连接
void handle_client(int client_socket) {
    char buffer[1024];
    int logged_in = 0;

    while (1) {
        memset(buffer, 0, sizeof(buffer));
        int bytes_received = recv(client_socket, buffer, sizeof(buffer) - 1, 0);
        if (bytes_received <= 0) {
            break;
        }

        // 处理 USER 命令
        if (strncmp(buffer, "USER ", 5) == 0) {
            handle_user_command(client_socket, buffer);
        }
        // 处理 PASS 命令
        else if (strncmp(buffer, "PASS ", 5) == 0) {
            logged_in = handle_pass_command(client_socket, buffer);
            if (logged_in) {
                break;  // 登录成功，退出循环
            }
        } else {
            send(client_socket, "500 Unknown command.\r\n", 22, 0);
        }
    }
}
