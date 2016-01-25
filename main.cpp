#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <cmath>
#include <string>
#include <sstream>
#include <string.h>
using namespace std;

#include "Server.h"

int main(void) {
  Server::init("1234");

  Server::loop();

  Server::close();
  return 0;
}