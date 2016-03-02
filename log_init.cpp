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
  printf("successful open\n");

  Log::finish();

  return 0;
}