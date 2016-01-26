#include "HTTP.h"

const char *HTTP::F_ADD_NODE = "/api/v1/add_node",
           *HTTP::F_ADD_EDGE = "/api/v1/add_edge";

void HTTP::request(struct mbuf *data) {
  struct http_message parsedRequest;

  // Parse the request.
  int msgSize = mg_parse_http(data->buf, (int)data->len, &parsedRequest, 1);
  if (msgSize <= 0) return;

  // Make method is POST.
  const struct mg_str& method = parsedRequest.method;
  if (strncmp(method.p, "POST", method.len) != 0) return;

  // Parse the JSON.
  const struct mg_str& body = parsedRequest.body;
  struct json_token *json = parse_json2(body.p, body.len);
  if (json == NULL) return;

  // Process different functions.
  const struct mg_str& uri = parsedRequest.uri;
  if (!strncmp(F_ADD_NODE, uri.p, uri.len)) {
    printf("Got add_node: %.*s\n", (int)uri.len, uri.p);
  } else if (!strncmp(F_ADD_EDGE, uri.p, uri.len)) {
    printf("Got add_edge: %.*s\n", (int)uri.len, uri.p);
  } else {
    printf("Got bad URI: %.*s\n", (int)uri.len, uri.p);
  }
}