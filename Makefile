CC = g++
CFLAGS  = -g -Wall -std=c++11 -pedantic -Wno-deprecated-register
MONGOOSE = mongoose.c

all: cs426_graph_server cs426_graph_client log_test

cs426_graph_server: main.cpp Server.cpp HTTP.cpp Graph.cpp
	$(CC) $(CFLAGS) -o cs426_graph_server main.cpp Server.cpp HTTP.cpp Graph.cpp $(MONGOOSE)

cs426_graph_client: client.cpp
	$(CC) $(CFLAGS) -o cs426_graph_client client.cpp $(MONGOOSE)

log_test: log_test.cpp Log.cpp
	$(CC) $(CFLAGS) -o log_test log_test.cpp Log.cpp

clean:
	rm cs426_graph_server
	rm cs426_graph_client
	rm log_test