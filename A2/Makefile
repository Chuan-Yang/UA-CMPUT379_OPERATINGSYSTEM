CC = gcc   

all:
	make client server

client: client.c
	$(CC) client.c -o wbc379 -lcrypto 

server: server.c
	$(CC) -pthread server.c -o wbs379
clean:
	rm -rf wbs379 wbc379