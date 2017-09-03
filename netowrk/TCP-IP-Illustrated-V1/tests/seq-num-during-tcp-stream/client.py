import socket

host = "127.0.0.1"
port = 12345

s = socket.socket(family=socket.AF_INET, type=socket.SOCK_STREAM, proto=0)

s.connect((host, port))

s.send("hello world")
s.send("blabla")
s.send("dsafdsaf")
