#include <stdio.h>
#include <string.h>
#include "auth.h"

int validate_user(const char *username) {
    if (strcmp(username, "anonymous") == 0) {
        return 1;
    }
    return 0;
}

int validate_password(const char *password) {
    if (strstr(password, "@") != NULL) {
        return 1;
    }
    return 0;
}

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
        return 1;
    } else {
        send(client_socket, "530 Invalid password.\r\n", 23, 0);
        return 0;
    }
}