import os
import sys

print("#ALL ENV VARS")
for key, value in sorted(os.environ.items()):
    print(f"{key} : {value}")
print("")

method = os.environ.get("REQUEST_METHOD", "")
query = os.environ.get("QUERY_STRING", "")

# POST body
length = int(os.environ.get("CONTENT_LENGTH", 0))
body = sys.stdin.read(length) if length > 0 else ""

print("<h1>CGI TEST</h1>")
print(f"<p>METHOD: '{method}'</p>")
print(f"<p>QUERY: '{query}' </p>")
print(f"<p>BODY: '{body}' </p>")
