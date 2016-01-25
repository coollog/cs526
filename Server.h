#include "mongoose.h"

class Server {
public:
  static void init(const char *port);
  static void loop();
  static void close();

private:
  static void handleEvent(struct mg_connection *nc, int type, void *data);

  // All events are handled by eventNAME functions.
  static void eventReceive(struct mg_connection *nc, void *data);

  static struct mg_mgr mgr;
};