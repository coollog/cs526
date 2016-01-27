#include "HTTP.h"

const char *HTTP::F_ADD_NODE = "/api/v1/add_node",
           *HTTP::F_ADD_EDGE = "/api/v1/add_edge",
           *HTTP::F_REMOVE_NODE = "/api/v1/remove_node",
           *HTTP::F_REMOVE_EDGE = "/api/v1/remove_edge",
           *HTTP::F_GET_NODE = "/api/v1/get_node",
           *HTTP::F_GET_EDGE = "/api/v1/get_edge",
           *HTTP::F_GET_NEIGHBORS = "/api/v1/get_neighbors",
           *HTTP::F_SHORTEST_PATH = "/api/v1/shortest_path";

const char *HTTP::RC_200_OK = "200 OK",
           *HTTP::RC_204_OK = "204 OK",
           *HTTP::RC_400_BAD_REQUEST = "400 Bad Request";

const unsigned int HTTP::JSON_MAX_LEN = 1000,
                   HTTP::REPLY_MAX_LEN = 1000;

Graph HTTP::graph;

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
  struct json_token *json = parseJsonFromHTTPMessage(data);
  if (json == NULL) return;

  // DEBUG CODE ONLY
  // graph.addNode(81);
  // graph.addNode(82);
  // graph.addEdge(81, 82);

  // Process different functions.
  const struct mg_str& uri = data->uri;

  const char *responseCode;

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
    responseCode = requestGetNode(json);

  } else if (!strncmp(F_GET_EDGE, uri.p, uri.len)) {
    printf("Got GET_EDGE: %.*s\n", (int)uri.len, uri.p);
    responseCode = requestGetEdge(json);

  } else if (!strncmp(F_GET_NEIGHBORS, uri.p, uri.len)) {
    printf("Got GET_NEIGHBORS: %.*s\n", (int)uri.len, uri.p);
    responseCode = requestGetNeighbors(json);

  } else if (!strncmp(F_SHORTEST_PATH, uri.p, uri.len)) {
    printf("Got SHORTEST_PATH: %.*s\n", (int)uri.len, uri.p);
    responseCode = requestShortestPath(json);

  } else {
    printf("Got bad URI: %.*s\n", (int)uri.len, uri.p);
    return;
  }

  char reply[REPLY_MAX_LEN];
  int replyLen;

  if (responseCode == RC_200_OK) {
    const struct mg_str& body = data->body;
    replyLen = sprintf(reply, "HTTP/1.1 %s\r\n"
                              "Content-Length: %lu\r\n"
                              "Content-Type: application/json\r\n"
                              "\r\n%.*s\r\n",
                              responseCode, body.len, (int)body.len, body.p);
  } else {
    replyLen = sprintf(reply, "HTTP/1.1 %s\r\n\r\n", responseCode);
  }

  mg_send(nc, reply, replyLen);

  printf("SENT:\n%.*s\n", replyLen, reply);
}

const char *HTTP::requestAddNode(struct json_token *json) {
  // Get the id.
  unsigned int id = tokenToInt(find_json_token(json, "node_id"));

  if (graph.addNode(id) == -1) {
    printf("ADD NODE EXISTING NODE\n");
    return RC_204_OK;
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
  }

  return RC_200_OK;
}

const char *HTTP::requestRemoveNode(struct json_token *json) {
  // Get the id.
  unsigned int id = tokenToInt(find_json_token(json, "node_id"));

  if (graph.removeNode(id) == -2) {
    printf("REMOVE NODE NONEXISTENT NODE\n");
    return RC_400_BAD_REQUEST;
  }

  return RC_200_OK;
}

const char *HTTP::requestRemoveEdge(struct json_token *json) {
  // Get the ids.
  unsigned int id_a = tokenToInt(find_json_token(json, "node_a_id"));
  unsigned int id_b = tokenToInt(find_json_token(json, "node_b_id"));

  if (graph.removeEdge(id_a, id_b) == -2) {
    printf("REMOVE EDGE NONEXISTENT EDGE\n");
    return RC_400_BAD_REQUEST;
  }

  return RC_200_OK;
}

const char *HTTP::requestGetNode(struct json_token *json) {
  return RC_200_OK;
}

const char *HTTP::requestGetEdge(struct json_token *json) {
  return RC_200_OK;
}

const char *HTTP::requestGetNeighbors(struct json_token *json) {
  return RC_200_OK;
}

const char *HTTP::requestShortestPath(struct json_token *json) {
  return RC_200_OK;
}
