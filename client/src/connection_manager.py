import socket
from command_sender import login
from data_transfer import handle_pasv, handle_port

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
        elif command.upper() == "PORT":
            handle_port(sock)
        else:
            pass


