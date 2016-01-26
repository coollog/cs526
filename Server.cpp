#include "Server.h"

struct mg_mgr Server::mgr;

void Server::init(const char *port) {
  mg_mgr_init(&mgr, NULL);

  // Listen on port
  mg_bind(&mgr, port, &Server::handleEvent);
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
    eventReceive(nc, data);
    break;

  case MG_EV_HTTP_REQUEST:
    eventHTTP(nc, data);
    break;

  default:
    break;
  }
}

void Server::eventReceive(struct mg_connection *nc, struct mbuf *data) {
  printf("MG_EV_RECV:\n%.*s\n", (int)data->len, data->buf);

  HTTP::request(nc, data);

  // Discard data from recv buffer.
  mbuf_remove(data, data->len);
}

void Server::eventHTTP(struct mg_connection *nc, struct mbuf *data) {
  printf("MG_EV_HTTP_REQUEST:\n%.*s\n", (int)data->len, data->buf);
}