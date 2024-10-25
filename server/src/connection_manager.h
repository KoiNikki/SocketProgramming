#ifndef CONNECTION_MANAGER_H
#define CONNECTION_MANAGER_H
#define MAX_CLIENTS 10

void handle_connections(int server_socket, int client_sockets[MAX_CLIENTS]);

#endif // CONNECTION_MANAGER_H
