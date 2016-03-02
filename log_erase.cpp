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

  Log::erase();
  printf("successful erase\n");

  Log::Metadata md = Log::getMetadata();
  printf("metadata generation: %u, size: %u\n",
         md.generation, md.size);

  Log::finish();

  return 0;
}