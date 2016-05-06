#include <string>
#include <iostream>
#include <cstdio>
#include <memory>

#include "mongoose.h"

class RPC {
public:
  static int sendWrite(const char *partitionHost,
                        const char *type,
                        unsigned int id1,
                        unsigned int id2);

private:
  static std::string executeCurl(const char *partitionHost,
                                 const char *type,
                                 unsigned int id1,
                                 unsigned int id2);
};