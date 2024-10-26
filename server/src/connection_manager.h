#ifndef CONNECTION_MANAGER_H
#define CONNECTION_MANAGER_H

#define MAX_CLIENTS 10
#define MAX_DIR_LEN 512

void handle_connections(int server_socket, int client_sockets[MAX_CLIENTS], const char *directory);

#endif // CONNECTION_MANAGER_H
