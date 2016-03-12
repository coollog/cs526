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

static const char *M_ADD_NODE_1 = "POST /api/v1/add_node HTTP/1.1\r\n"
                                  "charset: utf-8\r\n"
                                  "Content-Type: application/json\r\n"
                                  "User-Agent: Java/1.7.0_71\r\n"
                                  "Host: localhost:1234\r\n"
                                  "Accept: text/html, image/gif, image/jpeg, *;"
                                  " q=.2, */*; q=.2\r\n"
                                  "Connection: keep-alive\r\n"
                                  "Content-Length: 14\r\n"
                                  "\r\n{\"node_id\":71}\r\n",
                  *M_ADD_NODE_2 = "POST /api/v1/add_node HTTP/1.1\r\n"
                                  "charset: utf-8\r\n"
                                  "Content-Type: application/json\r\n"
                                  "User-Agent: Java/1.7.0_71\r\n"
                                  "Host: localhost:1234\r\n"
                                  "Accept: text/html, image/gif, image/jpeg, *;"
                                  " q=.2, */*; q=.2\r\n"
                                  "Connection: keep-alive\r\n"
                                  "Content-Length: 14\r\n"
                                  "\r\n{\"node_id\":33}\r\n",
                  *M_ADD_EDGE = "POST /api/v1/add_edge HTTP/1.1\r\n"
                                "charset: utf-8\r\n"
                                "Content-Type: application/json\r\n"
                                "User-Agent: Java/1.7.0_71\r\n"
                                "Host: localhost:1234\r\n"
                                "Accept: text/html, image/gif, image/jpeg, *; "
                                "q=.2, */*; q=.2\r\n"
                                "Connection: keep-alive\r\n"
                                "Content-Length: 32\r\n"
                                "\r\n{\"node_a_id\":33,\"node_b_id\":71}\r\n",
                  *M_CHECKPOINT = "POST /api/v1/checkpoint HTTP/1.1\r\n"
                                  "charset: utf-8\r\n"
                                  "Content-Type: application/json\r\n"
                                  "User-Agent: Java/1.7.0_71\r\n"
                                  "Host: localhost:1234\r\n"
                                  "Accept: text/html, image/gif, image/jpeg, *;"
                                  " q=.2, */*; q=.2\r\n"
                                  "Connection: keep-alive\r\n"
                                  "Content-Length: 0\r\n\r\n",
                  *M_REMOVE_NODE = "POST /api/v1/remove_node HTTP/1.1\r\n"
                                   "charset: utf-8\r\n"
                                   "Content-Type: application/json\r\n"
                                   "User-Agent: Java/1.7.0_71\r\n"
                                   "Host: localhost:1234\r\n"
                                   "Accept: text/html, image/gif, image/jpeg, *;"
                                   " q=.2, */*; q=.2\r\n"
                                   "Connection: keep-alive\r\n"
                                   "Content-Length: 14\r\n"
                                   "\r\n{\"node_id\":71}\r\n",
                  *M_REMOVE_EDGE = "POST /api/v1/remove_edge HTTP/1.1\r\n"
                                   "charset: utf-8\r\n"
                                   "Content-Type: application/json\r\n"
                                   "User-Agent: Java/1.7.0_71\r\n"
                                   "Host: localhost:1234\r\n"
                                   "Accept: text/html, image/gif, image/jpeg, "
                                   "*; q=.2, */*; q=.2\r\n"
                                   "Connection: keep-alive\r\n"
                                   "Content-Length: 31\r\n"
                                   "\r\n{\"node_a_id\":33,\"node_b_id\":71}\r\n";

static const char *s_target_address, *message;

static int s_exit_flag = 0;

static void ev_handler(struct mg_connection *nc, int ev, void *ev_data) {
  struct mbuf *io = &nc->recv_mbuf;
  int connect_status;

  switch (ev) {
    case MG_EV_CONNECT:
      connect_status = * (int *) ev_data;
      if (connect_status == 0) {
        printf("Connected to %s, sending request...\n", s_target_address);
        mg_printf(nc, "%s", message);

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

int main(int argc, char *argv[]) {
  s_target_address = argv[1];

  if (argc != 3) {
    printf("takes: add1, add2, addedge, removenode, removeedge, or checkpoint\n");
    return 1;
  }
  if (!strcmp(argv[2], "add1")) {
    message = M_ADD_NODE_1;
  } else if (!strcmp(argv[2], "add2")) {
    message = M_ADD_NODE_2;
  } else if (!strcmp(argv[2], "addedge")) {
    message = M_ADD_EDGE;
  } else if (!strcmp(argv[2], "removenode")) {
    message = M_REMOVE_NODE;
  } else if (!strcmp(argv[2], "removeedge")) {
    message = M_REMOVE_EDGE;
  } else if (!strcmp(argv[2], "checkpoint")) {
    message = M_CHECKPOINT;
  } else {
    printf("takes: add1, add2, addedge, removenode, removeedge, or checkpoint\n");
    return 1;
  }

  struct mg_mgr mgr;

  mg_mgr_init(&mgr, NULL);
  mg_connect(&mgr, s_target_address, ev_handler);

  printf("Starting client against %s\n", s_target_address);
  while (s_exit_flag == 0) {
    mg_mgr_poll(&mgr, 1000);
  }

  mg_mgr_free(&mgr);
  return 0;
}