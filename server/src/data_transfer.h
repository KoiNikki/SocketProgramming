#ifndef DATA_TRANSFER_H
#define DATA_TRANSFER_H

void handle_port_command(int client_socket, const char *buffer);

void handle_pasv_command(int client_socket);

void handle_list_command(int client_socket, const char *directory);

void handle_retr_command(int client_socket, const char *filename);

void handle_stor_command(int client_socket, const char *filename);

#endif // DATA_TRANSFER_H
