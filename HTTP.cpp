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

const unsigned int HTTP::JSON_MAX_LEN = 1000;

Graph HTTP::graph;

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

  if (responseCode == RC_200_OK) {
    const struct mg_str& body = data->body;
    mg_printf(nc, "HTTP/1.1 %s\r\n"
                  "Content-Length: %lu\r\n"
                  "Content-Type: application/json\r\n"
                  "\r\n%.*s",
                  responseCode, body.len, (int)body.len, body.p);
  } else {
    mg_printf(nc, "HTTP/1.1 %s\r\n", responseCode);
  }
}

const char *HTTP::requestAddNode(struct json_token *json) {
  // Get the id.
  const struct json_token *idToken = find_json_token(json, "node_id");
  unsigned int id = atoi(std::string(idToken->ptr, idToken->len).c_str());

  if (graph.addNode(id) == -1) return RC_204_OK;

  return RC_200_OK;
}

const char *HTTP::requestAddEdge(struct json_token *json) {
  return RC_200_OK;
}

const char *HTTP::requestRemoveNode(struct json_token *json) {
  return RC_200_OK;
}

const char *HTTP::requestRemoveEdge(struct json_token *json) {
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
