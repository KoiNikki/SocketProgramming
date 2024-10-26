#include <sys/socket.h>
#include <stdio.h>
#include <string.h>
#include "command_parser.h"
#include "data_transfer.h"
#include "auth.h"

// 处理客户端连接
void handle_client(int client_socket, const char *buffer, char *current_directory, const char *root_directory)
{
    // int logged_in = 0;

    // 处理 USER
    if (strncmp(buffer, "USER ", 5) == 0)
    {
        handle_user_command(client_socket, buffer);
    }
    // 处理 PASS
    else if (strncmp(buffer, "PASS ", 5) == 0)
    {
        handle_pass_command(client_socket, buffer);
    }
    // 处理 PORT
    else if (strncmp(buffer, "PORT ", 5) == 0)
    {
        handle_port_command(client_socket, buffer);
    }
    // 处理 PASV
    else if (strncmp(buffer, "PASV", 4) == 0)
    {
        handle_pasv_command(client_socket);
    }
    else if (strncmp(buffer, "LIST", 4) == 0)
    {
        handle_list_command(client_socket, current_directory);
    }
    else if (strncmp(buffer, "RETR ", 5) == 0)
    {
        char filename[256];
        sscanf(buffer, "RETR %s", filename);
        char filepath[512];
        snprintf(filepath, sizeof(filepath), "%s/%s", current_directory, filename);
        handle_retr_command(client_socket, filepath);
    }
    else if (strncmp(buffer, "STOR ", 5) == 0)
    {
        char filename[256];
        sscanf(buffer, "STOR %s", filename);
        char filepath[512];
        snprintf(filepath, sizeof(filepath), "%s/%s", current_directory, filename);
        handle_stor_command(client_socket, filepath);
    }
    else if (strncmp(buffer, "PWD", 3) == 0)
    {
        handle_pwd_command(client_socket, current_directory, root_directory);
    }
    else if (strncmp(buffer, "CWD ", 4) == 0)
    {
        handle_cwd_command(client_socket, buffer, current_directory, root_directory);
    }
    else if (strncmp(buffer, "MKD ", 4) == 0)
    {
        char dirname[256];
        sscanf(buffer, "MKD %s", dirname);
        char dirpath[512];
        snprintf(dirpath, sizeof(dirpath), "%s/%s", current_directory, dirname);
        handle_mkd_command(client_socket, dirpath);
    }
    else if (strncmp(buffer, "RMD ", 4) == 0)
    {
        char dirname[256];
        sscanf(buffer, "RMD %s", dirname);
        char dirpath[512];
        snprintf(dirpath, sizeof(dirpath), "%s/%s", current_directory, dirname);
        handle_rmd_command(client_socket, dirpath);
    }
    // 添加 SYST 命令处理
    else if (strncmp(buffer, "SYST", 4) == 0)
    {
        send(client_socket, "215 UNIX Type: L8\r\n", 19, 0);
    }
    else if (strncmp(buffer, "TYPE", 4) == 0)
    {
        // 添加 TYPE 命令处理
        if (strncmp(buffer, "TYPE I", 6) == 0)
        {
            send(client_socket, "200 Type set to I.\r\n", 20, 0);
        }
        else
        {
            send(client_socket, "504 Command not implemented for that parameter.\r\n", 49, 0);
        }
    }
    // 添加 QUIT 命令处理
    else if (strncmp(buffer, "QUIT", 4) == 0)
    {
        send(client_socket, "221 Goodbye.\r\n", 14, 0);
    }
    else
    {
        send(client_socket, "500 Unknown command.\r\n", 22, 0);
    }
}
