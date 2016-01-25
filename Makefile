CC = g++
CFLAGS  = -g -Wall -std=c++11 -pedantic -Wno-deprecated-register
MONGOOSE = mongoose.c

all: cs426_graph_server cs426_graph_client

cs426_graph_server: main.cpp Server.cpp HTTP.cpp
	$(CC) $(CFLAGS) -o cs426_graph_server main.cpp Server.cpp HTTP.cpp $(MONGOOSE)

cs426_graph_client: client.cpp
	$(CC) $(CFLAGS) -o cs426_graph_client client.cpp $(MONGOOSE)

clean:
	rm cs426_graph_server
	rm cs426_graph_client