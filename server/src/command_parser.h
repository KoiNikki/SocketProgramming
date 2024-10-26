#ifndef COMMAND_PARSER_H
#define COMMAND_PARSER_H

void handle_client(int client_socket, const char *buffer, char *current_directory, const char* root_directory);

#endif // COMMAND_PARSER_H
