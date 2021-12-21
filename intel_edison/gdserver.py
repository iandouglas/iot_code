import socket
import sys
import time
from thread import *

# Intel Edison I/O library
import mraa

led = mraa.Gpio(13)
led.dir(mraa.DIR_OUT)

relay = mraa.Gpio(3)
relay.dir(mraa.DIR_OUT)

HOST = ''   # Use all available interfaces
PORT = 8888 # Connect to app over this non-privileged port

PWD  = 'secret password'

s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
print 'Socket created'

s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)

# Bind socket to local host and port
try:
    s.bind((HOST, PORT))
except socket.error as msg:
    print 'Bind failed. Error Code : ' + str(msg[0]) + ' Message ' + msg[1]
    sys.exit()

print 'Socket bind complete'

# Start listening on socket
s.listen(10)
print 'Socket now listening'

# Function to blink and LED and activate the garage door relay
def activateDoorRelay():
    led.write(1)    # turn the LED on
    relay.write(1)  # turn the relay on
    time.sleep(1)   # delay for 1 second
    led.write(0)    # turn the LED off
    relay.write(0)  # turn the relay off

# Function for handling connections. This will be used to create threads
def clientthread(conn):
    # Sending message to connected client
    conn.send('Welcome to the server. Please enter the password\n')

    led.write(0)    # the default LED state is off
    relay.write(0)  # the default relay state is off

    # Loop until break exit
    while True:

        data = conn.recv(1024)
        data = data.rstrip()

        print 'received ' + data

        if data == 'exit':
            break
        elif data == PWD:
            reply = 'password OK\n'
            conn.sendall(reply)
            activateDoorRelay()
        else:
            reply = 'incorrect password\n'
            conn.sendall(reply)

    # break exit
    conn.close()

# Infinite loop to connect with clients
while True:
    # blocking call will wait to accept a connection
    conn, addr = s.accept()
    print 'Connected with ' + addr[0] + ':' + str(addr[1])
    start_new_thread(clientthread ,(conn,))

s.close()
