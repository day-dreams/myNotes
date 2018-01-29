import socket
import time
import sys

porta = int(sys.argv[1])
portb = int(sys.argv[2])

s = socket.socket(family=socket.AF_INET, type=socket.SOCK_STREAM, proto=0)

interfacea = ("127.0.0.1", porta)
interfaceb = ("127.0.0.1", portb)

s.bind(interfacea)
# s.listen(5)

s.connect(interfaceb)
# time.sleep(3)

print "connect stat:",s.connect(interfaceb)
