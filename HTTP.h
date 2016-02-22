#include <string>

#include "mongoose.h"
#include "Graph.h"

class HTTP {
public:
  static void request(struct mg_connection *nc, struct http_message *data);

private:
  static const char *F_ADD_NODE,
                    *F_ADD_EDGE,
                    *F_REMOVE_NODE,
                    *F_REMOVE_EDGE,
                    *F_GET_NODE,
                    *F_GET_EDGE,
                    *F_GET_NEIGHBORS,
                    *F_SHORTEST_PATH,
                    *F_CHECKPOINT;

  static const char *RC_200_OK,
                    *RC_204_OK,
                    *RC_400_BAD_REQUEST,
                    *RC_507_INSUFFICIENT_SPACE;

  static const unsigned int JSON_MAX_LEN, REPLY_MAX_LEN;

  static int tokenToInt(struct json_token *token);

  static bool checkMethodPOST(struct http_message *data);
  static struct json_token *parseJsonFromHTTPMessage(struct http_message *data);

  static const char *requestAddNode(struct json_token *json);
  static const char *requestAddEdge(struct json_token *json);
  static const char *requestRemoveNode(struct json_token *json);
  static const char *requestRemoveEdge(struct json_token *json);
  static const char *requestGetNode(struct json_token *json,
                                    char jsonBuf[],
                                    int *jsonLen);
  static const char *requestGetEdge(struct json_token *json,
                                    char jsonBuf[],
                                    int *jsonLen);
  static const char *requestGetNeighbors(struct json_token *json,
                                         char jsonBuf[],
                                         int *jsonLen);
  static const char *requestShortestPath(struct json_token *json,
                                         char jsonBuf[],
                                         int *jsonLen);
  static const char *requestCheckpoint(struct json_token *json);

  static Graph graph;
};