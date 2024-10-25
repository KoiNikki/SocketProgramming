import socket
from command_sender import login
from data_transfer import handle_pasv, handle_port, handle_list, handle_retr, handle_stor

def handle_connection():
    server_ip = '127.0.0.1'
    server_port = 2121

    # 创建socket并连接到服务器
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    sock.connect((server_ip, server_port))

    try:
        # 登录到FTP服务器
        if login(sock):
            # 登录成功后进入命令模式
            command_prompt(sock)
    finally:
        sock.close()

def command_prompt(sock):
    while True:
        command = input("ftp> ").strip()
        
        # 退出命令
        if command.lower() == 'quit':
            print("Exiting FTP client.")
            break
        
        # 根据命令调用不同的处理函数
        if command.upper() == "PASV":
            handle_pasv(sock)
        elif command.upper().startswith("PORT"):
            data_socket = handle_port(sock)
        elif command.upper() == "LIST":
            handle_list(sock, data_socket if 'data_socket' in locals() else None)
        elif command.upper().startswith("RETR"):
            _, filename = command.split(maxsplit=1)
            handle_retr(sock, filename, data_socket if 'data_socket' in locals() else None)
        elif command.upper().startswith("STOR"):
            _, filename = command.split(maxsplit=1)
            handle_stor(sock, filename, data_socket if 'data_socket' in locals() else None)
        else:
            print("Unknown command.")
            