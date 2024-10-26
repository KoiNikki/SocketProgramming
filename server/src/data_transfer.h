#ifndef DATA_TRANSFER_H
#define DATA_TRANSFER_H

void handle_port_command(int client_socket, const char *buffer);

void handle_pasv_command(int client_socket);

void handle_list_command(int client_socket, char *current_directory);

void handle_retr_command(int client_socket, const char *filename);

void handle_stor_command(int client_socket, const char *filename);

void handle_mkd_command(int client_socket, const char *dirpath);

void handle_pwd_command(int client_socket, const char *current_directory, const char *root_directory);

void handle_cwd_command(int client_socket, const char *buffer, char *current_directory, const char *root_directory);

void handle_rmd_command(int client_socket, const char *dirpath);

#endif // DATA_TRANSFER_H
