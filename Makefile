all: client server

client: client.c raw_socket.c network.h packet.c packet.h bitwise.h bitwise.c commands.c commands.h
	rm -rf client
	gcc -o client -Wall client.c raw_socket.c packet.c bitwise.c -g

server: server.c raw_socket.c network.h packet.c packet.h bitwise.h bitwise.c commands.c commands.h
	rm -rf server
	gcc -o server -Wall server.c raw_socket.c packet.c bitwise.c -g

clean:
	rm -rf server client a.out
	