#include <string>

#include "mongoose.h"
#include "Graph.h"

class HTTP {
public:
  static void request(struct mg_connection *nc, struct http_message *data);

private:
  static const char *F_ADD_NODE,
                    *F_ADD_EDGE,
                    *RC_200_OK,
                    *RC_204_OK,
                    *RC_400_BAD_REQUEST;
  static const unsigned int JSON_MAX_LEN;

  static Graph graph;
};