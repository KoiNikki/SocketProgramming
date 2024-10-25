#ifndef AUTH_H
#define AUTH_H

int validate_user(const char *username);

int validate_password(const char *password);

void handle_welcome(int client_socket);

void handle_user_command(int client_socket, const char *buffer);

int handle_pass_command(int client_socket, const char *buffer);

#endif // AUTH_H
