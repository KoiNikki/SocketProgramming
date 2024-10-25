#ifndef DATA_TRANSFER_H
#define DATA_TRANSFER_H

void handle_port_command(int client_socket, const char *buffer);

void handle_pasv_command(int client_socket);

#endif // DATA_TRANSFER_H
