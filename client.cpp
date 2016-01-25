#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <cmath>
#include <string>
#include <sstream>
#include <string.h>
using namespace std;

#include "mongoose.h"

static const char *s_target_address = "localhost:1234";

static int s_exit_flag = 0;

static void ev_handler(struct mg_connection *nc, int ev, void *ev_data) {
  struct mbuf *io = &nc->recv_mbuf;
  int connect_status;

  switch (ev) {
    case MG_EV_CONNECT:
      connect_status = * (int *) ev_data;
      if (connect_status == 0) {
        printf("Connected to %s, sending request...\n", s_target_address);
        // mg_printf(nc, "GET %s HTTP/1.0\r\nHost: %s\r\n\r\n",
        //           s_request, s_target_address);
      } else {
        printf("Error connecting to %s: %s\n",
               s_target_address, strerror(connect_status));
        s_exit_flag = 1;
      }
      break;
    case MG_EV_RECV:
      printf("Received: %.*s\n", (int)io->len, io->buf);
      mbuf_remove(io, io->len);      // Discard data from recv buffer
      break;
    default:
      break;
  }
}

int main(void) {
  struct mg_mgr mgr;
  struct mg_connection *nc;

  mg_mgr_init(&mgr, NULL);
  nc = mg_connect(&mgr, s_target_address, ev_handler);

  printf("Starting client against %s\n", s_target_address);
  while (s_exit_flag == 0) {
    mg_mgr_poll(&mgr, 1000);
  }

  mg_mgr_free(&mgr);
  return 0;
}