# OS HW3 – Multi-Client TCP Chat Server

Operating Systems course (TAU) – Homework 3.

## Overview

A multi-client TCP chat server and client in C using POSIX sockets and pthreads. The server accepts multiple simultaneous connections and relays messages between clients. Each client connection is handled in a dedicated thread.

**Key features:**
- TCP socket server supporting up to 100 concurrent clients
- Per-client pthread handler
- Graceful `SIGPIPE` handling for dropped connections
- Separate client program with send/receive threads

## Files

| File | Description |
|------|-------------|
| `hw3server.c` | Server: accepts connections, manages client table, broadcasts messages |
| `hw3client.c` | Client: connects to server, sends/receives messages |
| `hw3.h` | Shared constants, socket helpers, and error-check macro |
| `Makefile` | Build system |
| `solution.pdf` | Written solution document |

## Build & Run

```bash
make
```

Start the server:
```bash
./server <port>
```

Connect a client:
```bash
./client <server-ip> <port> <name>
```
