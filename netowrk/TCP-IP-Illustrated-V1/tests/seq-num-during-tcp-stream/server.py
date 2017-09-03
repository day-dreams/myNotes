import socket
import time

host = "127.0.0.1"
port = 12345


s = socket.sockets = socket.socket(
    family=socket.AF_INET, type=socket.SOCK_STREAM, proto=0)

s.bind((host, port))

s.listen(5)

client, addr = s.accept()


client.send("dsafadsf")
client.send("dsafadsf")
client.send("dsafadsf")
client.send("dsafadsf")
client.send("dsafadsf")
client.send("dsafadsf")
