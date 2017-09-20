import socket

s = socket.socket(family=socket.AF_INET, type=socket.SOCK_STREAM, proto=0)


s.bind(("0.0.0.0", 9999))

s.listen(5)

client, addr = s.accept()

request = client.recv(4)

print "[", request, "]"
