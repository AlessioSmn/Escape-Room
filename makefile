# make rule primaria con dummy target ‘all’--> non crea alcun file all ma fa un complete build
# che dipende dai target client e server scritti sotto
all: server client

client: ClientFiles/client.c common_utils.c
	gcc -Wall $^ -o client

server: ServerFiles/server.c ServerFiles/game_server.c ServerFiles/room_server.c ServerFiles/server_utils.c common_utils.c
	gcc -Wall $^ -o server

# pulizia dei file della compilazione (eseguito con ‘make clean’ da terminale)
clean:
	rm *o client server