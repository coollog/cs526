#include "HTTP.h"

static void tokenToStr(struct json_token *token, char *dest) {
  strcpy(dest, std::string(token->ptr, token->len).c_str());
}
static int tokenToInt(struct json_token *token) {
  return atoi(std::string(token->ptr, token->len).c_str());
}

const char *HTTP::F_ADD_NODE = "/api/v1/add_node",
           *HTTP::F_ADD_EDGE = "/api/v1/add_edge",
           *HTTP::F_REMOVE_EDGE = "/api/v1/remove_edge",
           *HTTP::F_GET_NODE = "/api/v1/get_node",
           *HTTP::F_GET_EDGE = "/api/v1/get_edge",
           *HTTP::F_GET_NEIGHBORS = "/api/v1/get_neighbors",
           *HTTP::F_RPC = "/rpc";

const char *HTTP::RC_200_OK = "200 OK",
           *HTTP::RC_204_OK = "204 OK",
           *HTTP::RC_400_BAD_REQUEST = "400 Bad Request";

const unsigned int HTTP::JSON_MAX_LEN = 1000,
                   HTTP::REPLY_MAX_LEN = 1000;

int HTTP::partitionIndex;
const char **HTTP::partitionList;

Graph HTTP::graph;

void HTTP::init(int partitionIndex, const char *partitionList[3]) {
  HTTP::partitionIndex = partitionIndex;
  HTTP::partitionList = partitionList;
}

bool HTTP::checkMethodPOST(struct http_message *data) {
  const struct mg_str& method = data->method;
  return strncmp(method.p, "POST", method.len) == 0;
}

struct json_token *HTTP::parseJsonFromHTTPMessage(struct http_message *data) {
  const struct mg_str& body = data->body;
  return parse_json2(body.p, body.len);
}

void HTTP::request(struct mg_connection *nc, struct http_message *data) {
  // Make sure method is POST.
  if (!checkMethodPOST(data)) return;

  // Parse the JSON.
  struct json_token *json;
  if (data->body.len > 0) {
    json = parseJsonFromHTTPMessage(data);
    if (json == NULL) {
      printf("ERROR PARSING JSON\n");
      return;
    }
  }

  // Process different functions.
  const struct mg_str& uri = data->uri;
  const struct mg_str& body = data->body;

  const char *responseCode;
  char jsonBuf[JSON_MAX_LEN];
  const char *responseBody = body.p;
  int responseLen = body.len;

  if (!strncmp(F_ADD_NODE, uri.p, uri.len)) {
    printf("Got ADD_NODE: %.*s\n", (int)uri.len, uri.p);
    responseCode = requestAddNode(json);

  } else if (!strncmp(F_ADD_EDGE, uri.p, uri.len)) {
    printf("Got ADD_EDGE: %.*s\n", (int)uri.len, uri.p);
    responseCode = requestAddEdge(json);

  } else if (!strncmp(F_REMOVE_EDGE, uri.p, uri.len)) {
    printf("Got REMOVE_EDGE: %.*s\n", (int)uri.len, uri.p);
    responseCode = requestRemoveEdge(json);

  } else if (!strncmp(F_GET_NODE, uri.p, uri.len)) {
    printf("Got GET_NODE: %.*s\n", (int)uri.len, uri.p);
    responseCode = requestGetNode(json, jsonBuf, &responseLen);
    responseBody = jsonBuf;

  } else if (!strncmp(F_GET_EDGE, uri.p, uri.len)) {
    printf("Got GET_EDGE: %.*s\n", (int)uri.len, uri.p);
    responseCode = requestGetEdge(json, jsonBuf, &responseLen);
    responseBody = jsonBuf;

  } else if (!strncmp(F_GET_NEIGHBORS, uri.p, uri.len)) {
    printf("Got GET_NEIGHBORS: %.*s\n", (int)uri.len, uri.p);
    responseCode = requestGetNeighbors(json, jsonBuf, &responseLen);
    responseBody = jsonBuf;

  } else if (!strncmp(F_RPC, uri.p, uri.len)) {
    printf("Got RPC: %.*s\n", (int)uri.len, uri.p);
    responseCode = requestRPC(responseBody, responseLen, jsonBuf);
    responseBody = jsonBuf;
    responseLen = (int)strlen(responseBody);

  } else {
    printf("Got bad URI: %.*s\n", (int)uri.len, uri.p);
    return;
  }

  char reply[REPLY_MAX_LEN];
  int replyLen;

  if (responseCode == RC_200_OK) {
    replyLen = sprintf(reply, "HTTP/1.1 %s\r\n"
                              "Content-Length: %d\r\n"
                              "Content-Type: application/json\r\n"
                              "\r\n%.*s\r\n",
                              responseCode,
                              responseLen,
                              responseLen, responseBody);
  } else {
    replyLen = sprintf(reply, "HTTP/1.1 %s\r\n"
                              "Content-Length: 0\r\n"
                              "Content-Type: application/json\r\n"
                              "\r\n\r\n", responseCode);
  }

  mg_send(nc, reply, replyLen);

  printf("SENT:\n%.*s\n", replyLen, reply);
}

const char *HTTP::requestAddNode(struct json_token *json) {
  // Get the id.
  unsigned int id = tokenToInt(find_json_token(json, "node_id"));

  switch (callPartition(F_ADD_NODE, id, 0)) {
  case -1:
    printf("ADD NODE EXISTING NODE\n");
    return RC_204_OK;
  case 0:
    return RC_200_OK;
  default:
    if (graph.addNode(id) == -1) {
      printf("ADD NODE EXISTING NODE\n");
      return RC_204_OK;
    }
  }

  printGraph();

  return RC_200_OK;
}

const char *HTTP::requestAddEdge(struct json_token *json) {
  // Get the ids.
  unsigned int id_a = tokenToInt(find_json_token(json, "node_a_id"));
  unsigned int id_b = tokenToInt(find_json_token(json, "node_b_id"));

  if (isIdMine(id_a)) {
    switch (graph.addEdge(id_a, id_b)) {
    case -1:
      printf("ADD EDGE INVALID\n");
      return RC_204_OK;
    case -2:
      printf("ADD EDGE BAD NODE\n");
      return RC_400_BAD_REQUEST;
    }
    printGraph();
  } else {
    switch (callPartition(F_ADD_EDGE, id_a, id_b)) {
    case -1:
      printf("ADD EDGE INVALID\n");
      return RC_204_OK;
    case -2:
      printf("ADD EDGE BAD NODE\n");
      return RC_400_BAD_REQUEST;
    }
  }

  if (isIdMine(id_b)) {
    switch (graph.addEdge(id_b, id_a)) {
    case -1:
      printf("ADD EDGE INVALID\n");
      return RC_204_OK;
    case -2:
      printf("ADD EDGE BAD NODE\n");
      return RC_400_BAD_REQUEST;
    }
    printGraph();
  } else {
    switch (callPartition(F_ADD_EDGE, id_b, id_a)) {
    case -1:
      printf("ADD EDGE INVALID\n");
      return RC_204_OK;
    case -2:
      printf("ADD EDGE BAD NODE\n");
      return RC_400_BAD_REQUEST;
    }
  }

  return RC_200_OK;
}

const char *HTTP::requestRemoveEdge(struct json_token *json) {
  // Get the ids.
  unsigned int id_a = tokenToInt(find_json_token(json, "node_a_id"));
  unsigned int id_b = tokenToInt(find_json_token(json, "node_b_id"));

  if (isIdMine(id_a)) {
    switch (graph.removeEdge(id_a, id_b)) {
    case -2:
      printf("REMOVE EDGE NONEXISTENT EDGE\n");
      return RC_400_BAD_REQUEST;
    }

    printGraph();
  } else {
    switch (callPartition(F_REMOVE_EDGE, id_a, id_b)) {
    case -2:
      printf("REMOVE EDGE NONEXISTENT EDGE\n");
      return RC_400_BAD_REQUEST;
    }
  }

  if (isIdMine(id_b)) {
    switch (graph.removeEdge(id_b, id_a)) {
    case -2:
      printf("REMOVE EDGE NONEXISTENT EDGE\n");
      return RC_400_BAD_REQUEST;
    }

    printGraph();
  } else {
    switch (callPartition(F_REMOVE_EDGE, id_b, id_a)) {
    case -2:
      printf("REMOVE EDGE NONEXISTENT EDGE\n");
      return RC_400_BAD_REQUEST;
    }
  }

  return RC_200_OK;
}

const char *HTTP::requestGetNode(struct json_token *json,
                                 char jsonBuf[],
                                 int *jsonLen) {
  // Get the id.
  unsigned int id = tokenToInt(find_json_token(json, "node_id"));

  int inGraph;

  switch (callPartition(F_GET_NODE, id, 0)) {
  case 0: inGraph = 0; break;
  case 1: inGraph = 1; break;
  default: inGraph = graph.getNode(id);
  }

  *jsonLen = json_emit(jsonBuf, JSON_MAX_LEN,
                       "{ s: i }", "in_graph", inGraph);

  return RC_200_OK;
}

const char *HTTP::requestGetEdge(struct json_token *json,
                                 char jsonBuf[],
                                 int *jsonLen) {
  // Get the ids.
  unsigned int id_a = tokenToInt(find_json_token(json, "node_a_id"));
  unsigned int id_b = tokenToInt(find_json_token(json, "node_b_id"));

  int inGraph = graph.getEdge(id_a, id_b);
  if (inGraph == -2) {
    printf("GET EDGE BAD NODE\n");
    return RC_400_BAD_REQUEST;
  }

  const char *format = inGraph ? "{ s: T }" : "{ s: F }";
  *jsonLen = json_emit(jsonBuf, JSON_MAX_LEN, format, "in_graph");

  return RC_200_OK;
}

const char *HTTP::requestGetNeighbors(struct json_token *json,
                                      char jsonBuf[],
                                      int *jsonLen) {
  // Get the id.
  unsigned int id = tokenToInt(find_json_token(json, "node_id"));

  if (isIdMine(id)) {
    Graph::IdSet *neighbors = graph.getNeighbors(id);
    if (neighbors == NULL) {
      printf("GET NEIGHBORS NONEXISTENT NODE\n");
      return RC_400_BAD_REQUEST;
    }

    *jsonLen = json_emit(jsonBuf, JSON_MAX_LEN,
                         "{ s: i, s: [", "node_id", id, "neighbors");
    for (unsigned int id: *neighbors) {
      *jsonLen += json_emit(jsonBuf + *jsonLen, JSON_MAX_LEN - *jsonLen,
                            "i, ", id);
    }
    if (neighbors->size() > 0) *jsonLen -= 2;
    *jsonLen += json_emit(jsonBuf + *jsonLen, JSON_MAX_LEN - *jsonLen, "] }");

    return RC_200_OK;
  }

  struct json_token *neighbors =
    RPC::sendWrite(partitionList[id % 3], F_GET_NEIGHBORS, id, 0);

  if (find_json_token(neighbors, "neighbors") == NULL) {
    printf("GET NEIGHBORS NONEXISTENT NODE\n");
    return RC_400_BAD_REQUEST;
  }

  *jsonLen = json_emit(jsonBuf, JSON_MAX_LEN,
                       "V", neighbors->ptr, neighbors->len);

  return RC_200_OK;
}

const char *HTTP::requestRPC(const char *jsonBody,
                             int jsonLen,
                             char jsonBuf[]) {
  static const char *methods[] = { "write", NULL };
  static mg_rpc_handler_t handlers[] = { HTTP::rpc_write, NULL };
  char buf[JSON_MAX_LEN];

  mg_rpc_dispatch(jsonBody, jsonLen, buf, JSON_MAX_LEN, methods, handlers);
  memcpy(jsonBuf, buf, JSON_MAX_LEN);

  return RC_200_OK;
}

bool HTTP::isIdMine(unsigned int id) {
  int targetPartition = id % 3;
  return targetPartition == partitionIndex;
}

int HTTP::callPartition(const char *type, unsigned int id1, unsigned int id2) {
  if (isIdMine(id1)) return 1337;

  int targetPartition = id1 % 3;
  return
    tokenToInt(RPC::sendWrite(partitionList[targetPartition], type, id1, id2));
}

int HTTP::rpc_write(char *buf, int len, struct mg_rpc_request *req) {
  char type[0x1000];
  tokenToStr(find_json_token(req->params, "type"), type);
  unsigned int id1 = tokenToInt(find_json_token(req->params, "id1"));
  unsigned int id2 = tokenToInt(find_json_token(req->params, "id2"));

  printf("Got rpc_write: %s, %u, %u\n", type, id1, id2);

  // Perform RPC.
  int returnCode = 0;

  if (!strcmp(F_ADD_NODE, type)) {
    returnCode = graph.addNode(id1);

  } else if (!strcmp(F_ADD_EDGE, type)) {
    returnCode = graph.addEdge(id1, id2);

  } else if (!strcmp(F_REMOVE_EDGE, type)) {
    returnCode = graph.removeEdge(id1, id2);

  } else if (!strcmp(F_GET_NODE, type)) {
    returnCode = graph.getNode(id1);

  } else if (!strcmp(F_GET_EDGE, type)) {
    returnCode = graph.getEdge(id1, id2);

  } else if (!strcmp(F_GET_NEIGHBORS, type)) {
    Graph::IdSet *neighbors = graph.getNeighbors(id1);

    if (neighbors == NULL) {
      printf("GET NEIGHBORS NONEXISTENT NODE\n");
      returnCode = -2;
    } else {
      char jsonBuf[JSON_MAX_LEN];
      int jsonLen;

      jsonLen = json_emit(jsonBuf, JSON_MAX_LEN,
                          "{ s: i, s: [", "node_id", id1, "neighbors");
      for (unsigned int id: *neighbors) {
        jsonLen += json_emit(jsonBuf + jsonLen, JSON_MAX_LEN - jsonLen,
                             "i, ", id);
      }
      if (neighbors->size() > 0) jsonLen -= 2;
      jsonLen += json_emit(jsonBuf + jsonLen, JSON_MAX_LEN - jsonLen, "] }");

      return mg_rpc_create_reply(buf, len, req, "V", jsonBuf, jsonLen);
    }
  }

  printGraph();

  return mg_rpc_create_reply(buf, len, req, "i", returnCode);
}

void HTTP::printGraph() {
  graph.print();
}