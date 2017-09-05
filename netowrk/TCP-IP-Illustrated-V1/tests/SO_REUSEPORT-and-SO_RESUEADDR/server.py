import socket
import time

host = "127.0.0.1"
port = 6666


s = socket.sockets = socket.socket(
    family=socket.AF_INET, type=socket.SOCK_STREAM, proto=0)

s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEPORT, 1)

s.bind((host, port))

s.listen(5)

while True:
    client, addr = s.accept()
    print "recv from:", addr
