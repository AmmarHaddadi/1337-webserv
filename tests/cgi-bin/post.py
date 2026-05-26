#!/usr/bin/env python3
import os
import sys

body = sys.stdin.read()

print("=== CGI POST TEST ===")
print("METHOD =", os.environ.get("REQUEST_METHOD"))
print("QUERY  =", os.environ.get("QUERY_STRING"))
print("LENGTH =", os.environ.get("CONTENT_LENGTH"))
print()

print("=== BODY ===")
print(body)
