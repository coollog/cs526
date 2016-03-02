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
  if (argc != 3) {
    printf("Usage: cs426_graph_server <port> <devfile>\n");
    return 1;
  }

  Server::init(argv[1], argv[2]);

  Server::loop();

  Server::close();
  return 0;
}