#include <arpa/inet.h>
#include "command_parser.h"

// 用于存储客户端指定的地址和端口
struct sockaddr_in client_data_addr;

void handle_port_command(int client_socket, const char *buffer) {
    int h1, h2, h3, h4, p1, p2;
    char ip[16];
    int port;

    // 解析PORT命令中的IP地址和端口
    if (sscanf(buffer, "PORT %d,%d,%d,%d,%d,%d", &h1, &h2, &h3, &h4, &p1, &p2) != 6) {
        send(client_socket, "500 Syntax error, command unrecognized.\r\n", 41, 0);
        return;
    }

    // 构造IP地址字符串
    snprintf(ip, sizeof(ip), "%d.%d.%d.%d", h1, h2, h3, h4);

    // 计算端口号
    port = p1 * 256 + p2;

    // 设置客户端数据地址结构
    client_data_addr.sin_family = AF_INET;
    client_data_addr.sin_port = htons(port);
    inet_pton(AF_INET, ip, &client_data_addr.sin_addr);

    // 发送成功响应
    send(client_socket, "200 PORT command successful.\r\n", 30, 0);
}

int passive_socket = -1;

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