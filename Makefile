CC = g++
CFLAGS  = -g -Wall -std=c++11 -pedantic -Wno-deprecated-register
MONGOOSE = mongoose.c
CPP = main.cpp Server.cpp HTTP.cpp Graph.cpp RPC.cpp

all: cs426_graph_server cs426_graph_client

cs426_graph_server: $(CPP)
	$(CC) $(CFLAGS) -o cs426_graph_server $(CPP) $(MONGOOSE)

cs426_graph_client: client.cpp
	$(CC) $(CFLAGS) -o cs426_graph_client client.cpp $(MONGOOSE)

clean:
	rm cs426_graph_server
	rm cs426_graph_client