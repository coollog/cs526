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

// const char *devFile = "/dev/sdc";
// const char *devFile = "/Users/coollog/Desktop/storage";

void printEntry(const Log::Entry& entry) {
  printf("entry: {%u, %llu, %llu}\n", entry.opCode, entry.id1, entry.id2);
}

int main(int argc, char *argv[]) {
  if (!Log::init(argv[1])) {
    printf("init error: %d\n", Log::getErrno());
    return 1;
  }
  printf("successful init\n");

  Log::Metadata md = Log::getMetadata();
  printf("metadata generation: %u, size: %u, cpsize: %u\n",
         md.generation, md.logSize, md.checkpointSize);

  if (!Log::add(1, 1234, 4321)) {
    printf("add error: %d\n", Log::getErrno());
    return 1;
  }
  printf("successful add\n");

  Log::moveToStart();

  Log::Entry entry;
  while (Log::playback(&entry)) {
    printEntry(entry);
  }
  printf("finished playback\n");

  Log::finish();

  return 0;
}