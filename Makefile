CC = gcc
CFLAGS = -Wall -g -pthread
# Using -pthread for both linker and compiler is best practice in Linux

all: hw3server hw3client

hw3server: hw3server.c hw3.h
	$(CC) $(CFLAGS) -o hw3server hw3server.c

hw3client: hw3client.c hw3.h
	$(CC) $(CFLAGS) -o hw3client hw3client.c

clean:
	rm -f hw3server hw3client