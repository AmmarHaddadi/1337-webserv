import os
import sys

method = os.environ.get("REQUEST_METHOD", "")
query = os.environ.get("QUERY_STRING", "")

# POST body
length = int(os.environ.get("CONTENT_LENGTH", 0))
body = sys.stdin.read(length) if length > 0 else ""

print("Content-Type: text/html")
print()

print("<h1>CGI TEST</h1>")
print("<p>METHOD:", method, "</p>")
print("<p>QUERY:", query, "</p>")
print("<p>BODY:", body, "</p>")