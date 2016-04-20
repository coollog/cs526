#include "Server.h"

struct mg_mgr Server::mgr;

void Server::init(const char *port) {
  mg_mgr_init(&mgr, NULL);

  // Listen on port.
  struct mg_connection *nc = mg_bind(&mgr, port, &Server::handleEvent);

  // Set to use HTTP protocol.
  mg_set_protocol_http_websocket(nc);
}

void Server::loop() {
  for (;;) {
    mg_mgr_poll(&mgr, 1000);
  }
}

void Server::close() {
  mg_mgr_free(&mgr);
}


void Server::handleEvent(struct mg_connection *nc, int type, void *evData) {
  struct mbuf *data = &nc->recv_mbuf;

  switch (type) {
  case MG_EV_RECV:
    // eventReceive(nc, data);
    break;

  case MG_EV_HTTP_REQUEST:
    eventHTTP(nc, data, (struct http_message *)evData);
    break;

  default:
    break;
  }
}

void Server::eventReceive(struct mg_connection *nc, struct mbuf *data) {
  printf("MG_EV_RECV:\n%.*s\n", (int)data->len, data->buf);

  // Discard data from recv buffer.
  mbuf_remove(data, data->len);
}

void Server::eventHTTP(struct mg_connection *nc,
                       struct mbuf *rawData,
                       struct http_message *data) {
  printf("MG_EV_HTTP_REQUEST:\n%.*s\n", (int)rawData->len, rawData->buf);

  HTTP::request(nc, data);
}