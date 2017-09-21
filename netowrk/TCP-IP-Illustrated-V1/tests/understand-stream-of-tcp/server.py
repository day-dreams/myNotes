import socket
import time
s = socket.socket(family=socket.AF_INET, type=socket.SOCK_STREAM, proto=0)


s.bind(("0.0.0.0", 10000 + 1 + 1 + 1 + 1))

s.listen(5)

client, addr = s.accept()

time.sleep(3)

for _ in range(30):
    request = client.recv(10)
    print "[", request, "]"
