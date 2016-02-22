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
  printf("successful open\n");

  if (!diskLog.readMetadata()) {
    printf("error: %d\n", diskLog.getErrno());
    return 1;
  }
  printf("successful metadata read\n");

  Log::Metadata md = diskLog.getMetadata();
  printf("metadata start: %d, size: %d, head: %d\n", md.start, md.size, md.head);

  if (!diskLog.reset()) {
    printf("error: %d\n", diskLog.getErrno());
    return 1;
  }
  printf("successful reset\n");

  Log::Metadata md = diskLog.getMetadata();
  printf("metadata start: %d, size: %d, head: %d\n", md.start, md.size, md.head);

  diskLog.finish();

  return 0;
}