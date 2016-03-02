#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <cmath>
#include <string>
#include <sstream>
#include <string.h>
using namespace std;

#include "Log.h"

int main(int argc, char *argv[]) {
  if (!Log::init("/dev/sdc")) {
    printf("error: %d\n", Log::getErrno());
    return 1;
  }

  if (!Log::erase()) {
    printf("error: %d\n", Log::getErrno());
    return 1;
  }
  printf("successful erase\n");

  Log::Metadata md = Log::getMetadata();
  printf("metadata generation: %u, size: %u, cpsize: %u\n",
         md.generation, md.logSize, md.checkpointSize);

  Log::finish();

  return 0;
}