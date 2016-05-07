#include "RPC.h"

struct json_token *RPC::sendWrite(const char *partitionHost,
                                  const char *type,
                                  unsigned int id1,
                                  unsigned int id2) {
  std::string response = executeCurl(partitionHost, type, id1, id2);
  const char *responseStr = response.c_str();
  printf("Got response:\n%s\n", responseStr);

  struct json_token *json = parse_json2(responseStr, strlen(responseStr));

  return find_json_token(json, "result");
}

std::string RPC::executeCurl(const char *partitionHost,
                             const char *type,
                             unsigned int id1,
                             unsigned int id2) {
  // Make the command.
  char cmd[0x1000];
  sprintf(
    cmd,
    "curl -d '{id:1,method:\"write\",params:{type:\"%s\",id1:%u,id2:%u}}' %s",
    type, id1, id2, partitionHost
  );

  printf("executing command: %s\n", cmd);

  std::shared_ptr<FILE> pipe(popen(cmd, "r"), pclose);
  if (!pipe) return "ERROR";

  char buffer[128];
  std::string result = "";
  while (!feof(pipe.get())) {
    if (fgets(buffer, 128, pipe.get()) != NULL)
      result += buffer;
  }
  return result;
}