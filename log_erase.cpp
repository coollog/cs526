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

  diskLog.erase();
  printf("successful erase\n");

  diskLog.finish();

  return 0;
}