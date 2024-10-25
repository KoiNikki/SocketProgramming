
def send_command(sock, command):
    sock.sendall(f"{command}\r\n".encode())
    response = sock.recv(1024).decode()
    return response

def login(sock):
    response = sock.recv(1024).decode()
    print(response)

    # username
    username = input("Please enter your username: ")
    response = send_command(sock, f"USER {username}")
    print(response)

    if "331" in response:
        # password
        email = input("Please enter your email address: ")
        response = send_command(sock, f"PASS {email}")
        print(response)

        if "230" in response:
            print("Login successful!")
            return True
        else:
            print("Login failed.")
            return False
    else:
        print("Invalid username.")
        return False
