import socket
import time

s = socket.socket(family=socket.AF_INET, type=socket.SOCK_STREAM, proto=0)

s.connect(('127.0.0.1', 9999))

s.send("hello world")
s.send("hello world")
s.send("hello world")
s.send("hello world")
s.send("hello world")
s.send("hello world")
s.send("hello world")

time.sleep(5)

s.send("hello world")
s.send("hello world")
s.send("hello world")
s.send("hello world")
s.send("hello world")
s.send("hello world")
