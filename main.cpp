#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <cmath>
#include <string>
#include <sstream>
#include <string.h>
using namespace std;

#include "Server.h"

int main(int argc, char *argv[]) {
  if (argc < 8 || strcmp("-p", argv[2]) || strcmp("-l", argv[4])) {
    printf("Usage: cs426_graph_server <port> -p <partnum> -l <partlist>\n");
    return 1;
  }

  const char *port = argv[1];
  int partitionIndex = atoi(argv[3]) - 1;

  char hostStr[3][0x1000];
  sprintf(hostStr[0], "%s/rpc", argv[5]);
  sprintf(hostStr[1], "%s/rpc", argv[6]);
  sprintf(hostStr[2], "%s/rpc", argv[7]);

  const char *partitionList[3] = { hostStr[0], hostStr[1], hostStr[2] };

  Server::init(port, partitionIndex, partitionList);

  // RPC::sendWrite("asdf", 1, 2);

  Server::loop();

  Server::close();
  return 0;
}