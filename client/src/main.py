import socket
from command_sender import login

def main():
    server_ip = '127.0.0.1'
    server_port = 2121

    # 创建socket并连接到服务器
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    sock.connect((server_ip, server_port))

    try:
        # 登录到FTP服务器
        login(sock)
    finally:
        sock.close()

if __name__ == "__main__":
    main()
