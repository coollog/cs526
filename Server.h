#include "mongoose.h"
#include "HTTP.h"

class Server {
public:
  static void init(const char *port);
  static void loop();
  static void close();

private:
  static void handleEvent(struct mg_connection *nc, int type, void *evData);

  // All events are handled by eventNAME functions.
  static void eventReceive(struct mg_connection *nc, struct mbuf *data);
  static void eventHTTP(struct mg_connection *nc,
                        struct mbuf *rawData,
                        struct http_message *data);

  static struct mg_mgr mgr;
};