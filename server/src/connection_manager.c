#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "connection_manager.h"
#include "auth.h"

void handle_client(int client_socket) {
    char buffer[1024];
    int logged_in = 0;

    // 发送欢迎信息
    send(client_socket, "220 Anonymous FTP server ready.\r\n", 34, 0);

    while (1) {
        memset(buffer, 0, sizeof(buffer));
        int bytes_received = recv(client_socket, buffer, sizeof(buffer) - 1, 0);
        if (bytes_received <= 0) {
            break;
        }

        // 处理USER命令
        if (strncmp(buffer, "USER ", 5) == 0) {
            char username[100];
            sscanf(buffer, "USER %s", username);
            if (validate_user(username)) {
                send(client_socket, "331 Please specify the password.\r\n", 34, 0);
            } else {
                send(client_socket, "530 Invalid username.\r\n", 23, 0);
            }
        }
        // 处理PASS命令
        else if (strncmp(buffer, "PASS ", 5) == 0) {
            char password[100];
            sscanf(buffer, "PASS %s", password);
            if (validate_password(password)) {
                send(client_socket, "230 Login successful.\r\n", 23, 0);
                logged_in = 1;
                break;  // 登录成功，退出循环
            } else {
                send(client_socket, "530 Invalid password.\r\n", 23, 0);
            }
        } else {
            send(client_socket, "500 Unknown command.\r\n", 22, 0);
        }
    }
}
