all: server client

server: tcp_server.cpp common.cpp cache.cpp
	g++ -ggdb -Wall tcp_server.cpp common.cpp cache.cpp -o tcp_server

client: tcp_client.cpp common.cpp
	g++ -ggdb -Wall tcp_client.cpp common.cpp -o tcp_client

clean:
	rm -rf tcp_server tcp_client tcp_server.dSYM tcp_client.dSYM