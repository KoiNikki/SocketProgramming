import socket

def send_command(sock, command):
    sock.sendall(f"{command}\r\n".encode())
    response = sock.recv(1024).decode()
    return response

def login(sock):
    response = sock.recv(1024).decode()
    print(response)  # 打印服务器的欢迎信息

    # 发送用户名
    response = send_command(sock, "USER anonymous")
    print(response)

    if "331" in response:
        # 发送密码（邮箱地址）
        email = input("Please enter your email address: ")
        response = send_command(sock, f"PASS {email}")
        print(response)

        if "230" in response:
            print("Login successful!")
        else:
            print("Login failed.")
    else:
        print("Invalid username.")
