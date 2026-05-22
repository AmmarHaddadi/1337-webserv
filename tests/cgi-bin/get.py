#!/usr/bin/env python3
import os

print("Content-Type: text/plain")
print()

print("=== CGI GET TEST ===")
print("METHOD =", os.environ.get("REQUEST_METHOD"))
print("QUERY  =", os.environ.get("QUERY_STRING"))
print()

print("=== ENV VARS ===")
for k, v in sorted(os.environ.items()):
    print(k, "=", v)
