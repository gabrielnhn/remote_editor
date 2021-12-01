default: all

all: client server


client: client.c raw_socket.c network.h packet.h bitwise.h
	gcc -o client -Wall client.c raw_socket.c packet.c bitwise.c


server: server.c raw_socket.c network.h packet.h bitwise.h
	gcc -o server -Wall server.c raw_socket.c packet.c bitwise.c

clean:
	rm -rf server client a.out
	