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
  if (argc < 2) {
    printf("Usage: cs426_graph_server <port> [-b <next node>]\n");
    return 1;
  }

  char hostStr[0x1000];
  char *nextNode = NULL;
  if (argc >= 5 && !strcmp("-b", argv[2])) {
    sprintf(hostStr, "%s:%s/rpc", argv[3], argv[4]);
    nextNode = hostStr;
  }

  Server::init(argv[1], nextNode);

  // RPC::sendWrite("asdf", 1, 2);

  Server::loop();

  Server::close();
  return 0;
}