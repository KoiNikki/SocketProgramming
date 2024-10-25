#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <errno.h>
#include "connection_manager.h"
#include "auth.h"

#define MAX_CLIENTS 10
#define BUFFER_SIZE 1024

void run_server(int port)
{
    int server_socket, client_socket, max_sd, activity, new_socket, sd;
    int client_sockets[MAX_CLIENTS] = {0};
    struct sockaddr_in server_addr, client_addr;
    fd_set readfds;
    socklen_t addr_len = sizeof(client_addr);

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

    while (1)
    {
        // 清空并设置文件描述符集
        FD_ZERO(&readfds);
        FD_SET(server_socket, &readfds);
        max_sd = server_socket;

        // 添加所有客户端套接字到文件描述符集中
        for (int i = 0; i < MAX_CLIENTS; i++)
        {
            sd = client_sockets[i];

            // 如果套接字有效，添加到读文件描述符集
            if (sd > 0)
            {
                FD_SET(sd, &readfds);
            }

            // 更新最大文件描述符
            if (sd > max_sd)
            {
                max_sd = sd;
            }
        }

        // 使用 select 监听文件描述符活动
        activity = select(max_sd + 1, &readfds, NULL, NULL, NULL);

        if ((activity < 0) && (errno != EINTR))
        {
            printf("select error");
        }

        // 检查是否有新的客户端连接
        if (FD_ISSET(server_socket, &readfds))
        {
            if ((new_socket = accept(server_socket, (struct sockaddr *)&client_addr, &addr_len)) < 0)
            {
                perror("accept");
                exit(EXIT_FAILURE);
            }

            printf("New connection: socket fd is %d , ip is : %s , port : %d\n",
                   new_socket, inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

            // 将新套接字添加到客户端数组中
            for (int i = 0; i < MAX_CLIENTS; i++)
            {
                if (client_sockets[i] == 0)
                {
                    client_sockets[i] = new_socket;
                    printf("Adding new client to list as %d\n", i);
                    break;
                }
            }
        }

        // 处理所有现有客户端的数据
        for (int i = 0; i < MAX_CLIENTS; i++)
        {
            sd = client_sockets[i];

            if (FD_ISSET(sd, &readfds))
            {
                // 客户端发送了数据，处理该客户端的请求
                handle_client(sd);

                // 如果客户端关闭了连接，关闭套接字并移除
                close(sd);
                client_sockets[i] = 0;
            }
        }
    }
}

int main(int argc, char *argv[])
{
    int port = 21;
    run_server(port);
    return 0;
}
