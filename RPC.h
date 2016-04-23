#include <string>
#include <iostream>
#include <cstdio>
#include <memory>

#include "mongoose.h"

class RPC {
public:
  static void init(const char *nextNode);

  static bool sendWrite(const char *type, unsigned int id1, unsigned int id2);

private:
  static std::string executeCurl(const char *type,
                                 unsigned int id1,
                                 unsigned int id2);

  static const char *nextHost;
};