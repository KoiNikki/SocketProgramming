import socket

size = 8192

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.bind(('', 9876))

index = 0

try:
  while True:
    data, address = sock.recvfrom(size)
    index += 1
    res = f"{index} {data.decode()}"
    sock.sendto(res.encode(), address)
finally:
  sock.close()