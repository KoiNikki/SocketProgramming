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
