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
    if data_socket:
        # PORT 模式
        handle_list_port(sock, data_socket)
    else:
        # PASV 模式
        handle_list_pasv(sock)

def handle_list_pasv(sock):
    # 使用 PASV 模式
    server_ip, server_port = handle_pasv(sock)
    if not server_ip or not server_port:
        print("Failed to enter PASV mode.")
        return

    data_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    try:
        data_socket.connect((server_ip, server_port))
    except Exception as e:
        print(f"Error connecting to PASV data socket: {e}")
        data_socket.close()
        return

    # 发送 LIST 命令
    response = send_command(sock, "LIST")
    print(response)

    if "150" in response:
        # 接收并打印目录列表
        receive_list_data(data_socket)
        response = sock.recv(1024).decode()
        print(response)

def handle_list_port(sock, data_socket):
    # PORT 模式下，直接发送 LIST 指令
    response = send_command(sock, "LIST")
    print(response)

    if "150" in response:
        # 接收并打印目录列表
        client_socket, _ = data_socket.accept()
        receive_list_data(client_socket)
        client_socket.close()

        response = sock.recv(1024).decode()
        print(response)

def receive_list_data(data_socket):
    """通用接收列表数据的函数"""
    try:
        while True:
            data = data_socket.recv(1024).decode()
            if not data:
                break
            print(data)
    finally:
        data_socket.close()

def handle_retr(sock, filename, data_socket=None):
    if data_socket:
        # PORT 模式
        handle_retr_port(sock, filename, data_socket)
    else:
        # PASV 模式
        handle_retr_pasv(sock, filename)

def handle_stor(sock, filename, data_socket=None):
    if data_socket:
        # PORT 模式
        handle_stor_port(sock, filename, data_socket)
    else:
        # PASV 模式
        handle_stor_pasv(sock, filename)

def handle_retr_pasv(sock, filename):
    # 使用 PASV 模式
    server_ip, server_port = handle_pasv(sock)
    if not server_ip or not server_port:
        print("Failed to enter PASV mode.")
        return

    data_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    try:
        data_socket.connect((server_ip, server_port))
    except Exception as e:
        print(f"Error connecting to PASV data socket: {e}")
        data_socket.close()
        return

    # 发送 RETR 命令
    response = send_command(sock, f"RETR {filename}")
    print(response)

    if "150" in response:
        # 接收文件数据并保存
        receive_file_data(data_socket, filename)
        response = sock.recv(1024).decode()
        print(response)

def handle_retr_port(sock, filename, data_socket):
    # PORT 模式下发送 RETR 命令
    response = send_command(sock, f"RETR {filename}")
    print(response)

    if "150" in response:
        # 接受服务器的连接并接收文件数据
        client_socket, _ = data_socket.accept()
        receive_file_data(client_socket, filename)
        client_socket.close()

        response = sock.recv(1024).decode()
        print(response)

def handle_stor_pasv(sock, filename):
    # 使用 PASV 模式
    server_ip, server_port = handle_pasv(sock)
    if not server_ip or not server_port:
        print("Failed to enter PASV mode.")
        return

    data_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    try:
        data_socket.connect((server_ip, server_port))
    except Exception as e:
        print(f"Error connecting to PASV data socket: {e}")
        data_socket.close()
        return

    # 发送 STOR 命令
    response = send_command(sock, f"STOR {filename}")
    print(response)

    if "150" in response:
        # 发送文件数据
        send_file_data(data_socket, filename)
        response = sock.recv(1024).decode()
        print(response)

def handle_stor_port(sock, filename, data_socket):
    # PORT 模式下发送 STOR 命令
    response = send_command(sock, f"STOR {filename}")
    print(response)

    if "150" in response:
        # 接受服务器的连接并发送文件数据
        client_socket, _ = data_socket.accept()
        send_file_data(client_socket, filename)
        client_socket.close()

        response = sock.recv(1024).decode()
        print(response)

def receive_file_data(data_socket, filename):
    """从数据连接中接收文件数据并保存"""
    try:
        with open(filename, "wb") as f:
            while True:
                data = data_socket.recv(1024)
                if not data:
                    break
                f.write(data)
    finally:
        data_socket.close()

def send_file_data(data_socket, filename):
    """从本地文件读取数据并发送到数据连接"""
    if not os.path.isfile(filename):
        print(f"File '{filename}' not found.")
        data_socket.close()
        return

    try:
        with open(filename, "rb") as f:
            while (data := f.read(1024)):
                data_socket.sendall(data)
    finally:
        data_socket.close()
