import socket
from command_sender import login, handle_quit, handle_syst, handle_type
from data_transfer import (
    handle_pasv,
    handle_port,
    handle_list,
    handle_retr,
    handle_stor,
    handle_mkd,
    handle_cwd,
    handle_pwd,
    handle_rmd,
)

data_socket = None


def handle_connection(server_ip, server_port):

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
    global data_socket
    while True:
        command = input("ftp> ").strip()

        # 退出命令
        if command.lower() == "quit":
            handle_quit(sock)
            break

        # 根据命令调用不同的处理函数
        if command.upper() == "PASV":
            handle_pasv(sock)
            data_socket = None
        elif command.upper().startswith("PORT"):
            data_socket = handle_port(sock)
        elif command.upper() == "LIST":
            handle_list(sock, data_socket)
        elif command.upper().startswith("RETR"):
            _, filename = command.split(maxsplit=1)
            handle_retr(sock, filename, data_socket)
        elif command.upper().startswith("STOR"):
            _, filename = command.split(maxsplit=1)
            handle_stor(sock, filename, data_socket)
        elif command.upper().startswith("MKD"):
            _, dirname = command.split(maxsplit=1)
            handle_mkd(sock, dirname)
        elif command.upper().startswith("CWD"):
            _, dirname = command.split(maxsplit=1)
            handle_cwd(sock, dirname)
        elif command.upper() == "PWD":
            handle_pwd(sock)
        elif command.upper().startswith("RMD"):
            _, dirname = command.split(maxsplit=1)
            handle_rmd(sock, dirname)
        elif command.upper() == "SYST":
            handle_syst(sock)
        elif command.upper().startswith("TYPE"):
            handle_type(sock, command)
        else:
            print("Unknown command.")
