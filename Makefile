default: all

all: client server


client: client.c raw_socket.c network.h
	gcc -o client -Wall client.c raw_socket.c


server: server.c raw_socket.c network.h
	gcc -o server -Wall server.c raw_socket.c

clean:
	rm -rf server client a.out
	