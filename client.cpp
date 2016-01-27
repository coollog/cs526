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
        // mg_printf(nc, "POST /api/v1/add_node HTTP/1.1\r\n"
        //               "charset: utf-8\r\n"
        //               "Content-Type: application/json\r\n"
        //               "User-Agent: Java/1.7.0_71\r\n"
        //               "Host: localhost:1234\r\n"
        //               "Accept: text/html, image/gif, image/jpeg, *; q=.2, */*; q=.2\r\n"
        //               "Connection: keep-alive\r\n"
        //               "Content-Length: 14\r\n"
        //               "\r\n{\"node_id\":71}\r\n");
        // mg_printf(nc, "POST /api/v1/add_node HTTP/1.1\r\n"
        //               "charset: utf-8\r\n"
        //               "Content-Type: application/json\r\n"
        //               "User-Agent: Java/1.7.0_71\r\n"
        //               "Host: localhost:1234\r\n"
        //               "Accept: text/html, image/gif, image/jpeg, *; q=.2, */*; q=.2\r\n"
        //               "Connection: keep-alive\r\n"
        //               "Content-Length: 14\r\n"
        //               "\r\n{\"node_id\":71}\r\n");
        mg_printf(nc, "POST /api/v1/remove_edge HTTP/1.1\r\n"
                      "charset: utf-8\r\n"
                      "Content-Type: application/json\r\n"
                      "User-Agent: Java/1.7.0_71\r\n"
                      "Host: localhost:1234\r\n"
                      "Accept: text/html, image/gif, image/jpeg, *; q=.2, */*; q=.2\r\n"
                      "Connection: keep-alive\r\n"
                      "Content-Length: 31\r\n"
                      "\r\n{\"node_a_id\":81,\"node_b_id\":82}\r\n");
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