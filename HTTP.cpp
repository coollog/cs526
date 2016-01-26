#include "HTTP.h"

const char *HTTP::F_ADD_NODE = "/api/v1/add_node",
           *HTTP::F_ADD_EDGE = "/api/v1/add_edge",
           *HTTP::RC_200_OK = "200 OK",
           *HTTP::RC_204_OK = "204 OK",
           *HTTP::RC_400_BAD_REQUEST = "400 Bad Request";
const unsigned int HTTP::JSON_MAX_LEN = 1000;

Graph HTTP::graph;

void HTTP::request(struct mg_connection *nc, struct http_message *data) {
  // Make sure method is POST.
  const struct mg_str& method = data->method;
  if (strncmp(method.p, "POST", method.len) != 0) return;

  // Parse the JSON.
  const struct mg_str& body = data->body;
  struct json_token *json = parse_json2(body.p, body.len);
  if (json == NULL) return;

  // Process different functions.
  const struct mg_str& uri = data->uri;

  const char *responseCode;
  char responseBody[JSON_MAX_LEN];
  int responseLen;

  if (!strncmp(F_ADD_NODE, uri.p, uri.len)) {
    printf("Got add_node: %.*s\n", (int)uri.len, uri.p);

    // Get the id.
    const struct json_token *idToken = find_json_token(json, "node_id");
    unsigned int id = atoi(std::string(idToken->ptr, idToken->len).c_str());

    if (graph.addNode(id)) {
      responseCode = RC_204_OK;
    } else {
      responseCode = RC_200_OK;
      responseLen = json_emit(responseBody, JSON_MAX_LEN,
                "{ s: i }", "node_id", id);
    }
  } else if (!strncmp(F_ADD_EDGE, uri.p, uri.len)) {
    printf("Got add_edge: %.*s\n", (int)uri.len, uri.p);
    responseCode = RC_200_OK;
  } else {
    printf("Got bad URI: %.*s\n", (int)uri.len, uri.p);
    return;
  }

  if (responseCode == RC_200_OK) {
    mg_printf(nc, "HTTP/1.1 %s\r\n"
                  "Content-Length: %d\r\n"
                  "Content-Type: application/json\r\n"
                  "\r\n%s",
                  responseCode, responseLen, responseBody);
  } else {
    mg_printf(nc, "HTTP/1.1 %s\r\n", responseCode);
  }
}