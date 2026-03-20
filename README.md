# TCP Multi-Client Chat Server in C

A real-time multi-client chat application built in C using POSIX sockets and pthreads.  
Supports up to 100 simultaneous clients, broadcast messaging, and private whisper messages.

## Architecture

- **Server** — thread-per-client model; each connection gets a dedicated pthread
- **Client** — two threads: one for sending (keyboard input), one for receiving (server messages)
- **Mutex-protected** client table prevents race conditions on the shared client list

## Features

- Broadcast: messages sent to all connected clients
- Whisper: private messages via `@username message`
- Graceful `SIGPIPE` handling for dropped connections
- Up to 100 concurrent clients

## Build & Run
```bash
make
```

Start the server:
```bash
./hw3server <port>
```

Connect a client:
```bash
./hw3client <server-ip> <port> <name>
```

## Example
```
./hw3server 8080

./hw3client 127.0.0.1 8080 Alice
./hw3client 127.0.0.1 8080 Bob

Alice: hello everyone
Bob: @Alice hey, private message!
```
