all:
	gcc -Wall -c common.c
	gcc -Wall client.c common.o -o client
	gcc -Wall server.c common.o -lpthread -lrt -o server

clean:
	rm common.o client server