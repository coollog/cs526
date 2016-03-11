CC = g++
CFLAGS  = -g -Wall -std=c++11 -pedantic -Wno-deprecated-register
MONGOOSE = mongoose.c
LOG = Log.cpp Log.Disk.cpp Log.BlockBuffer.cpp

all: cs426_graph_server cs426_graph_client log_test log_init log_erase

cs426_graph_server: main.cpp Server.cpp HTTP.cpp Graph.cpp $(LOG)
	$(CC) $(CFLAGS) -o cs426_graph_server main.cpp Server.cpp HTTP.cpp Graph.cpp $(LOG) $(MONGOOSE)

cs426_graph_client: client.cpp $(LOG)
	$(CC) $(CFLAGS) -o cs426_graph_client client.cpp $(LOG) $(MONGOOSE)

log_test: log_test.cpp $(LOG)
	$(CC) $(CFLAGS) -o log_test log_test.cpp $(LOG)
log_init: log_init.cpp $(LOG)
	$(CC) $(CFLAGS) -o log_init log_init.cpp $(LOG)
log_erase: log_erase.cpp $(LOG)
	$(CC) $(CFLAGS) -o log_erase log_erase.cpp $(LOG)

clean:
	rm cs426_graph_server cs426_graph_client log_test log_init log_erase