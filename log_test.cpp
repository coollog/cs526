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

void init(const char *filename) {
  if (!Log::init(filename)) {
    printf("init error: %d\n", Log::getErrno());
    exit(1);
  }
  printf("successful init\n");
}

void printEntry(const Log::Entry& entry) {
  printf("entry: {%u, %llu, %llu}\n", entry.opCode, entry.id1, entry.id2);
}

void printMetadata(const Log::Metadata& md) {
  printf("metadata generation: %u, size: %u, cpsize: %u\n",
         md.generation, md.logSize, md.checkpointSize);
}

void playback() {
  Log::moveToStart();

  Log::Entry entry;
  while (Log::playback(&entry)) {
    printEntry(entry);
  }
  printf("finished playback\n");
}

void add(uint32_t opCode, uint64_t id1, uint64_t id2) {
  if (!Log::add(opCode, id1, id2)) {
    printf("add error: %d\n", Log::getErrno());
    exit(1);
  }
  printf("successful add\n");
}

int main(int argc, char *argv[]) {
  init(argv[1]);

  printMetadata(Log::getMetadata());

  playback();

  if (argc > 2) {
    if (!strcmp(argv[2], "add")) {
      add(1, 1234, 4321);

      playback();
    }
  }

  Log::finish();

  return 0;
}