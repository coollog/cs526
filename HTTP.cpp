#include "HTTP.h"

const char *HTTP::F_ADD_NODE = "/api/v1/add_node",
           *HTTP::F_ADD_EDGE = "/api/v1/add_edge",
           *HTTP::F_REMOVE_NODE = "/api/v1/remove_node",
           *HTTP::F_REMOVE_EDGE = "/api/v1/remove_edge",
           *HTTP::F_GET_NODE = "/api/v1/get_node",
           *HTTP::F_GET_EDGE = "/api/v1/get_edge",
           *HTTP::F_GET_NEIGHBORS = "/api/v1/get_neighbors",
           *HTTP::F_SHORTEST_PATH = "/api/v1/shortest_path",
           *HTTP::F_CHECKPOINT = "/api/v1/checkpoint";

const char *HTTP::RC_200_OK = "200 OK",
           *HTTP::RC_204_OK = "204 OK",
           *HTTP::RC_400_BAD_REQUEST = "400 Bad Request",
           *HTTP::RC_507_INSUFFICIENT_SPACE = "507 Insufficient Space";

const unsigned int HTTP::JSON_MAX_LEN = 1000,
                   HTTP::REPLY_MAX_LEN = 1000;

Graph HTTP::graph;

void HTTP::init() {
  graph.init();
}

int HTTP::tokenToInt(struct json_token *token) {
  return atoi(std::string(token->ptr, token->len).c_str());
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

  } else if (!strncmp(F_REMOVE_NODE, uri.p, uri.len)) {
    printf("Got REMOVE_NODE: %.*s\n", (int)uri.len, uri.p);
    responseCode = requestRemoveNode(json);

  } else if (!strncmp(F_REMOVE_EDGE, uri.p, uri.len)) {
    printf("Got REMOVE_EDGE: %.*s\n", (int)uri.len, uri.p);
    responseCode = requestRemoveEdge(json);

  } else if (!strncmp(F_GET_NODE, uri.p, uri.len)) {
    printf("Got GET_NODE: %.*s\n", (int)uri.len, uri.p);
    responseCode = requestGetNode(json, jsonBuf, &responseLen);

  } else if (!strncmp(F_GET_EDGE, uri.p, uri.len)) {
    printf("Got GET_EDGE: %.*s\n", (int)uri.len, uri.p);
    responseCode = requestGetEdge(json, jsonBuf, &responseLen);

  } else if (!strncmp(F_GET_NEIGHBORS, uri.p, uri.len)) {
    printf("Got GET_NEIGHBORS: %.*s\n", (int)uri.len, uri.p);
    responseCode = requestGetNeighbors(json, jsonBuf, &responseLen);
    responseBody = jsonBuf;

  } else if (!strncmp(F_SHORTEST_PATH, uri.p, uri.len)) {
    printf("Got SHORTEST_PATH: %.*s\n", (int)uri.len, uri.p);
    responseCode = requestShortestPath(json, jsonBuf, &responseLen);
    responseBody = jsonBuf;

  } else if (!strncmp(F_CHECKPOINT, uri.p, uri.len)) {
    printf("Got CHECKPOINT: %.*s\n", (int)uri.len, uri.p);
    responseCode = requestCheckpoint();

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
    replyLen = sprintf(reply, "HTTP/1.1 %s\r\n\r\n", responseCode);
  }

  mg_send(nc, reply, replyLen);

  printf("SENT:\n%.*s\n", replyLen, reply);
}

const char *HTTP::requestAddNode(struct json_token *json) {
  // Get the id.
  unsigned int id = tokenToInt(find_json_token(json, "node_id"));

  switch (graph.addNode(id)) {
  case -1:
    printf("ADD NODE EXISTING NODE\n");
    return RC_204_OK;
  case -2:
    printf("ADD NODE LOGGING FAILED\n");
    return RC_507_INSUFFICIENT_SPACE;
  }

  return RC_200_OK;
}

const char *HTTP::requestAddEdge(struct json_token *json) {
  // Get the ids.
  unsigned int id_a = tokenToInt(find_json_token(json, "node_a_id"));
  unsigned int id_b = tokenToInt(find_json_token(json, "node_b_id"));

  switch (graph.addEdge(id_a, id_b)) {
  case -1:
    printf("ADD EDGE INVALID\n");
    return RC_204_OK;
  case -2:
    printf("ADD EDGE BAD NODE\n");
    return RC_400_BAD_REQUEST;
  case -3:
    printf("ADD EDGE LOGGING FAILED\n");
    return RC_507_INSUFFICIENT_SPACE;
  }

  return RC_200_OK;
}

const char *HTTP::requestRemoveNode(struct json_token *json) {
  // Get the id.
  unsigned int id = tokenToInt(find_json_token(json, "node_id"));

  switch (graph.removeNode(id)) {
  case -2:
    printf("REMOVE NODE NONEXISTENT NODE\n");
    return RC_400_BAD_REQUEST;
  case -3:
    printf("REMOVE NODE LOGGING FAILED\n");
    return RC_507_INSUFFICIENT_SPACE;
  }

  return RC_200_OK;
}

const char *HTTP::requestRemoveEdge(struct json_token *json) {
  // Get the ids.
  unsigned int id_a = tokenToInt(find_json_token(json, "node_a_id"));
  unsigned int id_b = tokenToInt(find_json_token(json, "node_b_id"));

  switch (graph.removeEdge(id_a, id_b)) {
  case -2:
    printf("REMOVE EDGE NONEXISTENT EDGE\n");
    return RC_400_BAD_REQUEST;
  case -3:
    printf("REMOVE EDGE LOGGING FAILED\n");
    return RC_507_INSUFFICIENT_SPACE;
  }

  return RC_200_OK;
}

const char *HTTP::requestGetNode(struct json_token *json,
                                 char jsonBuf[],
                                 int *jsonLen) {
  // Get the id.
  unsigned int id = tokenToInt(find_json_token(json, "node_id"));

  bool inGraph = graph.getNode(id);

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

const char *HTTP::requestShortestPath(struct json_token *json,
                                      char jsonBuf[],
                                      int *jsonLen) {
  // Get the ids.
  unsigned int id_a = tokenToInt(find_json_token(json, "node_a_id"));
  unsigned int id_b = tokenToInt(find_json_token(json, "node_b_id"));

  int dist = graph.shortestPath(id_a, id_b);
  switch (dist) {
  case -1:
    printf("SHORTEST PATH NO PATH\n");
    return RC_204_OK;
  case -2:
    printf("SHORTEST PATH NONEXISTENT NODE\n");
    return RC_400_BAD_REQUEST;
  }

  *jsonLen = json_emit(jsonBuf, JSON_MAX_LEN,
                       "{ s: i }", "distance", dist);

  return RC_200_OK;
}

const char *HTTP::requestCheckpoint() {
  if (!graph.checkpoint()) {
    printf("CHECKPOINT INSUFFICIENT SPACE\n");
    return RC_507_INSUFFICIENT_SPACE;
  }

  return RC_200_OK;
}
