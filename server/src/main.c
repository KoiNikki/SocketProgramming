#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <getopt.h>
#include "connection_manager.h"

void run_server(int port, const char *directory)
{
    int server_socket;
    int client_sockets[MAX_CLIENTS] = {0};
    struct sockaddr_in server_addr;

    // create socket
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0)
    {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // init address
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);

    // bind socket
    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("Bind failed");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    // listen
    if (listen(server_socket, 3) < 0)
    {
        perror("Listen failed");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %d\n", port);

    handle_connections(server_socket, client_sockets, directory);
}

int main(int argc, char *argv[])
{
    // int port = 21;
    int port = 2121;
    const char *directory = "tmp";
    int opt;

    // 使用 getopt 解析命令行参数
    while ((opt = getopt(argc, argv, "r:")) != -1) {
        switch (opt) {
            case 'r':  // 如果遇到 -root
                directory = optarg;  // 获取文件夹路径
                break;
            default:
                fprintf(stderr, "Usage: %s [-r root_directory]\n", argv[0]);
                exit(EXIT_FAILURE);
        }
    }

    run_server(port, directory);
    return 0;
}
