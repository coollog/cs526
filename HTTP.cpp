#include "HTTP.h"

void HTTP::request(struct mbuf *data) {
  struct http_message parsedRequest;

  // Parse the request.
  int msgSize = mg_parse_http(data->buf, (int)data->len, &parsedRequest, 1);
  if (msgSize <= 0) return;

  // Make method is POST.
  const struct mg_str& method = parsedRequest.method;
  if (strncmp(method.p, "POST", method.len) != 0) return;

  // Process different functions.
}