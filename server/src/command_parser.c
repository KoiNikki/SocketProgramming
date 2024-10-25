#include <sys/socket.h>
#include "command_parser.h"
#include "data_transfer.h"

// 处理客户端连接
void handle_client(int client_socket, const char *buffer)
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
    else
    {
        send(client_socket, "500 Unknown command.\r\n", 22, 0);
    }
}
