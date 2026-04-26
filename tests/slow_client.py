import socket
import time

# Configuration
HOST = '127.0.0.1'
PORT = 8080

s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.connect((HOST, PORT))

# Send only the FIRST part of the request
print("Sending partial request...")
s.send(b"GET / HTTP/1.1\r\nHost: localhost")
# Note: No \r\n\r\n yet! The server should be waiting.

print("Sleeping for 10 seconds. Try to curl the server NOW in another terminal!")
time.sleep(10)

# Send the final part to complete the headers
print("Sending the rest...")
s.send(b"\r\n\r\n")

# Read the response
response = s.recv(4096)
print("Response received:\n", response.decode())
s.close()
