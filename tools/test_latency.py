import signal
import socket
import sys
import time

running = True
DEF_CMD = "KRDG? A"


def usage(code: int):
    print(f"   Usage: {sys.argv[0]} <host> <port> [command]")
    print()
    print("                        host: hostname to connect to")
    print("                        port: tcp port to connect to (typically 7777)")
    print("          command (optional): which command to send repeatedely.")
    print(f'                              If not specified, "{DEF_CMD}" will be used')
    print()
    print(f"Examples: {sys.argv[0]} 10.0.0.1 7777")
    print(f'          {sys.argv[0]} 10.0.0.1 7777 "KRDG? A"')

    exit(code)


def sig_handler(sig, frame):
    global running
    running = False


if __name__ == "__main__":
    if len(sys.argv) < 3 or len(sys.argv) > 4:
        usage(1)

    host = sys.argv[1]
    if not host:
        usage(1)

    port = 0
    try:
        port = int(sys.argv[2])
    except Exception:
        usage(1)

    command = sys.argv[3] if len(sys.argv) > 3 else DEF_CMD
    if not command:
        usage(1)
    command += "\r\n"
    command = command.encode()

    signal.signal(signal.SIGINT, sig_handler)

    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
        # s.setsockopt(socket.IPPROTO_TCP, socket.TCP_NODELAY, 1)
        # s.setsockopt(socket.IPPROTO_TCP, socket.TCP_QUICKACK, 1)
        s.connect((host, port))
        while running:
            t = time.time()
            s.sendall(command)
            data = s.recv(32)
            while b"\r\n" not in data:
                data += s.recv(32)
            elapsed = time.time() - t
            elapsed = elapsed * 1000.0
            print(
                f"{elapsed:.3f} ms   {command.decode().strip()} -> {data.decode().strip()}"
            )
