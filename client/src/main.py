from connection_manager import handle_connection

server_ip = "127.0.0.1"
server_port = 21

if __name__ == "__main__":
    handle_connection(server_ip, server_port)
