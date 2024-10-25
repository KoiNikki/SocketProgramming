#include <sys/socket.h>
#include "command_parser.h"
#include "data_transfer.h"

// 处理客户端连接
void handle_client(int client_socket, const char *buffer, const char *directory)
{
    int logged_in = 0;

    // 处理 USER
    if (strncmp(buffer, "USER ", 5) == 0)
    {
        handle_user_command(client_socket, buffer);
    }
    // 处理 PASS
    else if (strncmp(buffer, "PASS ", 5) == 0)
    {
        logged_in = handle_pass_command(client_socket, buffer);
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
        handle_list_command(client_socket, directory);
    }
    else if (strncmp(buffer, "RETR ", 5) == 0)
    {
        char filename[256];
        sscanf(buffer, "RETR %s", filename);
        char filepath[512];
        snprintf(filepath, sizeof(filepath), "%s/%s", directory, filename);
        handle_retr_command(client_socket, filepath);
    }
    else if (strncmp(buffer, "STOR ", 5) == 0)
    {
        char filename[256];
        sscanf(buffer, "STOR %s", filename);
        char filepath[512];
        snprintf(filepath, sizeof(filepath), "%s/%s", directory, filename);
        handle_stor_command(client_socket, filepath);
    }
    else if (strncmp(buffer, "MKD ", 4) == 0)
    {
        char dirname[256];
        sscanf(buffer, "MKD %s", dirname);
        handle_mkd_command(client_socket, dirname);
    }
    else if (strncmp(buffer, "CWD ", 4) == 0)
    {
        char new_directory[256];
        sscanf(buffer, "CWD %s", new_directory);
        handle_cwd_command(client_socket, new_directory);
    }
    else if (strncmp(buffer, "PWD", 3) == 0)
    {
        handle_pwd_command(client_socket, directory);
    }
    else if (strncmp(buffer, "RMD ", 4) == 0)
    {
        char dirname[256];
        sscanf(buffer, "RMD %s", dirname);
        handle_rmd_command(client_socket, dirname);
    }
    else
    {
        send(client_socket, "500 Unknown command.\r\n", 22, 0);
    }
}
