# fast compilatioon for server and client programs.

all: server client

server: src/server.c src/mechanics.c
	gcc -Iinclude src/server.c src/mechanics.c -o server

client: src/client.c src/mechanics.c
	gcc -Iinclude src/client.c src/mechanics.c -o client

clean:
	rm -f server client