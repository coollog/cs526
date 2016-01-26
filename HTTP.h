#include "mongoose.h"

class HTTP {
public:
  static void request(struct mbuf *data);

private:
  static const char *F_ADD_NODE,
                    *F_ADD_EDGE;
};