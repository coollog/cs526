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
  struct mbuf *buf = &nc->recv_mbuf;

  switch (type) {
  case MG_EV_RECV:
    eventReceive(nc, buf);
    break;

  default:
    break;
  }
}

void Server::eventReceive(struct mg_connection *nc, struct mbuf *buf) {
  // This event handler implements simple TCP echo server
  mg_send(nc, buf->buf, buf->len);  // Echo received data back
  mbuf_remove(buf, buf->len);       // Discard data from recv buffer
}