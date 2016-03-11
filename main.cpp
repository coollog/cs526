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
  if (argc != 3 && argc != 4) {
    printf("Usage: cs426_graph_server [-f] <port> <devfile>\n");
    return 1;
  }

  bool format = false;
  const char *port;
  const char *devFile;

  // Process arguments.
  if (!strcmp(argv[1], "-f")) {
    format = true;
    port = argv[2];
    devFile = argv[3];
  } else {
    port = argv[1];
    devFile = argv[2];
  }
  // Init log.
  if (!Log::init(devFile)) {
    printf("Could not initialize log on '%s'\n", devFile);
    return 1;
  }
  // Format log if has -f flag.
  if (format) {
    if (!Log::erase()) {
      printf("Could not format fresh log.\n");
      return 1;
    }
    printf("Formatted fresh log.\n");
  }

  Server::init(port);

  Server::loop();

  Server::close();
  return 0;
}