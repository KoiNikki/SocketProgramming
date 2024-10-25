import os
import socket
import random
from command_sender import send_command

def handle_port(sock):
    # 获取本地IP地址并选择一个随机端口
    local_ip = sock.getsockname()[0]
    data_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    data_port = random.randint(20000, 65535)

    # 绑定并监听此随机端口
    try:
        data_socket.bind((local_ip, data_port))
        data_socket.listen(1)
    except Exception as e:
        print(f"Failed to bind or listen on the data port: {e}")
        data_socket.close()
        return None

    # 将IP和端口转换为FTP PORT命令格式
    ip_parts = local_ip.split('.')
    p1 = data_port // 256
    p2 = data_port % 256
    port_command = f"PORT {','.join(ip_parts)},{p1},{p2}"

    # 发送 PORT 命令
    response = send_command(sock, port_command)
    print(response)

    if "200" in response:
        print(f"Data socket listening on {local_ip}:{data_port}")
        return data_socket  # 返回数据套接字
    else:
        data_socket.close()
        return None

def handle_pasv(sock):
    # 发送 PASV 命令
    response = send_command(sock, "PASV")
    print(response)

    # 解析服务器返回的 IP 和端口
    if "227" in response:
        # 提取IP地址和端口号
        start = response.find('(') + 1
        end = response.find(')')
        ip_port_info = response[start:end].split(',')
        server_ip = '.'.join(ip_port_info[:4])
        server_port = int(ip_port_info[4]) * 256 + int(ip_port_info[5])

        print(f"PASV mode: Connect to {server_ip}:{server_port}")
        return server_ip, server_port
    else:
        print("Failed to enter PASV mode.")
        return None, None

def handle_list(sock, data_socket=None):
    # 如果在PORT模式下使用，接收一个数据套接字
    if not data_socket:
        server_ip, server_port = handle_pasv(sock)
        data_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        data_socket.connect((server_ip, server_port))

    # 发送 LIST 命令
    response = send_command(sock, "LIST")
    print(response)

    if "150" in response:
        # 接收并打印目录列表
        while True:
            data = data_socket.recv(1024).decode()
            if not data:
                break
            print(data)

        data_socket.close()
        response = sock.recv(1024).decode()
        print(response)

def handle_retr(sock, filename, data_socket=None):
    # 如果在PORT模式下使用，接收一个数据套接字
    if not data_socket:
        server_ip, server_port = handle_pasv(sock)
        data_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        data_socket.connect((server_ip, server_port))

    # 发送 RETR 命令
    response = send_command(sock, f"RETR {filename}")
    print(response)

    if "150" in response:
        # 打开本地文件以写入模式保存接收到的数据
        with open(filename, "wb") as f:
            while True:
                data = data_socket.recv(1024)
                if not data:
                    break
                f.write(data)

        data_socket.close()
        response = sock.recv(1024).decode()
        print(response)
    else:
        print("Failed to retrieve file.")

def handle_stor(sock, filename, data_socket=None):
    # 如果在PORT模式下使用，接收一个数据套接字
    if not data_socket:
        server_ip, server_port = handle_pasv(sock)
        data_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        data_socket.connect((server_ip, server_port))

    # 检查文件是否存在
    if not os.path.isfile(filename):
        print(f"File '{filename}' not found.")
        return

    # 发送 STOR 命令
    response = send_command(sock, f"STOR {filename}")
    print(response)

    if "150" in response:
        # 打开文件并将其内容发送到服务器
        with open(filename, "rb") as f:
            while (data := f.read(1024)):
                data_socket.sendall(data)

        data_socket.close()
        response = sock.recv(1024).decode()
        print(response)
    else:
        print("Failed to store file.")