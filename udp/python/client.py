import socket
 
size = 8192
sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
 
try:
  for i in range(51):
    req = f"message {i}"
    sock.sendto(req.encode(), ('localhost', 9876))
    res = sock.recv(size).decode()
    print(res)
 
except:
  print("cannot reach the server")
  
finally:
  sock.close()