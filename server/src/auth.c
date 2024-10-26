#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include "auth.h"
#include "data_transfer.h"

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
    const char *welcome_message = "220 Anonymous FTP server ready.\r\n";
    send_response(client_socket, welcome_message);
}

// 处理 USER 命令
void handle_user_command(int client_socket, const char *buffer) {
    char username[100];
    sscanf(buffer, "USER %s", username);
    if (validate_user(username)) {
        const char *response = "331 Please provide the password.\r\n";
        send_response(client_socket, response);
    } else {
        const char *response = "530 Invalid username.\r\n";
        send_response(client_socket, response);
    }
}

// 处理 PASS 命令
int handle_pass_command(int client_socket, const char *buffer) {
    char password[100];
    sscanf(buffer, "PASS %s", password);
    if (validate_password(password)) {
        const char *response = "230 Login successful.\r\n";
        send_response(client_socket, response);
        return 1;
    } else {
        const char *response = "530 Invalid password.\r\n";
        send_response(client_socket, response);
        return 0;
    }
}