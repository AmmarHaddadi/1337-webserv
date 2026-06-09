import argparse
import socket
import sys
import time


def send_pieces(sock, pieces, delay):
    for piece in pieces:
        sock.sendall(piece)
        time.sleep(delay)

def recv_response(sock):
    response = b""
    header_end = -1
    content_length = None
    sock.settimeout(5.0)
    while True:
        try:
            chunk = sock.recv(4096)
        except socket.timeout:
            break
        if not chunk:
            break
        response += chunk
        if header_end == -1:
            header_end = response.find(b"\r\n\r\n")
            if header_end != -1:
                headers = response[:header_end].decode("iso-8859-1", "replace")
                for line in headers.split("\r\n")[1:]:
                    if ":" not in line:
                        continue
                    name, value = line.split(":", 1)
                    if name.strip().lower() == "content-length":
                        try:
                            content_length = int(value.strip())
                        except ValueError:
                            content_length = None
                        break
        if header_end != -1 and content_length is not None:
            body_len = len(response) - (header_end + 4)
            if body_len >= content_length:
                break
    return response


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--host", default="127.0.0.1")
    parser.add_argument("--port", type=int, default=8080)
    parser.add_argument("--path", default="/")
    parser.add_argument("--delay", type=float, default=0.2)
    parser.add_argument("--keep-alive", action="store_true")
    args = parser.parse_args()

    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    sock.connect((args.host, args.port))

    body_pieces = [
        b"4\r\nW",
        b"iki\r\n5\r\np",
        b"edia\r\ne\r\n in\r\n",
        b"\r\nchunks.\r\n0\r\n",
        b"\r\n",
    ]
    expected_body = b"Wikipedia in\r\n\r\nchunks."
    connection_value = "keep-alive" if args.keep_alive else "close"

    headers = (
        (
            "POST {path} HTTP/1.1\r\n"
            "Host: {host}\r\n"
            "Transfer-Encoding: chunked\r\n"
            "Content-Type: text/plain\r\n"
            "Connection: {connection}\r\n"
            "\r\n"
        )
        .format(path=args.path, host=args.host, connection=connection_value)

        .encode()
    )

    send_pieces(sock, [headers] + body_pieces, args.delay)

    response = recv_response(sock)

    if not response:
        print("no response received", file=sys.stderr)
        return 1

    head = response.split(b"\r\n", 1)[0].decode("utf-8", "replace")
    print(head)
    if head.startswith("HTTP/1.1 400"):
        print(response.decode("utf-8", "replace"), file=sys.stderr)
        return 1
    parts = response.split(b"\r\n\r\n", 1)
    if len(parts) != 2:
        print("response missing header/body separator", file=sys.stderr)
        return 1
    if expected_body not in parts[1]:
        print("response body did not contain expected payload", file=sys.stderr)
        sock.close()
        return 1

    if args.keep_alive:
        follow_up = (
            "GET {path} HTTP/1.1\r\n"
            "Host: {host}\r\n"
            "Connection: close\r\n"
            "\r\n"
        ).format(path=args.path, host=args.host).encode()
        try:
            sock.sendall(follow_up)
        except socket.error:
            print("keep-alive connection closed before follow-up request", file=sys.stderr)
            sock.close()
            return 1
        follow_response = recv_response(sock)
        if not follow_response:
            print("no response to keep-alive follow-up request", file=sys.stderr)
            sock.close()
            return 1

    sock.close()
    return 0


if __name__ == "__main__":
    raise SystemExit(main())

# Quick manual test guide (temporary C++ echo):
# 1) In src/http/response/respond.cpp, inside respondToReq(), add:
#      if (req.method == POST) {
#          sMeta.responseBuf = generateHttpResponse(OK, req.body);
#          return;
#      }
# 2) Rebuild and run the server:
#      make && ./webserv
# 3) Run this client:
#      python3 tests/chunked_request.py --path /upload --delay 0.2
#    You should see: HTTP/1.1 200 ...
# 4) Remove the temporary C++ echo block after verification.
