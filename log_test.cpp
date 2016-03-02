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
  Log diskLog;

  if (!diskLog.init()) {
    printf("error: %d\n", diskLog.getErrno());
    return 1;
  }
  printf("successful init\n");

  Log::Metadata md = diskLog.getMetadata();
  printf("metadata generation: %u, size: %u\n",
         md.generation, md.size);

  diskLog.finish();

  return 0;
}