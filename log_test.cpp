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

  if (!diskLog.diskOpen()) return;
  printf("successful open\n");

  if (!diskLog.readMetadata()) return;
  printf("successful metadata read\n");

  return 0;
}