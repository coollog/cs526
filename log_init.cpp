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

  if (!diskLog.reset(100, Log::RESET_VERBOSE)) {
    printf("error: %d\n", diskLog.getErrno());
    return 1;
  }

  diskLog.finish();

  return 0;
}