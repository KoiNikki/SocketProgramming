#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>
#include <linux/limits.h>
#include "connection_manager.h"
#include "auth.h"
#include "command_parser.h"

void handle_connections(int server_socket, int client_sockets[MAX_CLIENTS], const char *directory)
{
    int max_sd, activity, new_socket, sd;
    struct sockaddr_in client_addr;
    fd_set readfds;
    socklen_t addr_len = sizeof(client_addr);
    char buffer[1024];

    char root_directory[PATH_MAX];

    // 将传入的 directory 转换为绝对路径
    if (realpath(directory, root_directory) == NULL) {
        perror("Failed to resolve root directory absolute path");
        exit(EXIT_FAILURE);
    }
    printf("Root directory set to: %s\n", root_directory);

    // 为每个客户端分配一个当前目录的数组
    char client_dirs[MAX_CLIENTS][MAX_DIR_LEN];

    // 初始化每个客户端的当前目录为服务器的初始目录
    for (int i = 0; i < MAX_CLIENTS; i++)
    {
        strncpy(client_dirs[i], root_directory, MAX_DIR_LEN);
        client_sockets[i] = 0;
    }

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

            // 发送欢迎信息
            handle_welcome(new_socket);

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
                // 清空缓冲区
                memset(buffer, 0, sizeof(buffer));
                int bytes_received = recv(sd, buffer, sizeof(buffer) - 1, 0);

                // 检查客户端是否关闭连接
                if (bytes_received == 0 || bytes_received == -1)
                {
                    // 客户端关闭连接或发生错误
                    printf("Client disconnected: fd %d\n", sd);
                    close(sd);
                    client_sockets[i] = 0;
                }
                else
                {
                    // 客户端发送了数据，处理该客户端的请求
                    handle_client(sd, buffer, client_dirs[i], root_directory);
                }
            }
        }
    }
}
