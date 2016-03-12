#include <cstdint>
#include <climits>
#include <unordered_set>
#include <set>
#include <unordered_map>
#include <utility>
#include <vector>
#include <assert.h>
#include <stdio.h>

#include "Log.h"

class Graph {
public:
  typedef std::unordered_set<uint64_t> IdSet;
  typedef std::unordered_map<uint64_t, IdSet> NodeList;

  void init();

  int addNode(uint64_t id);
  int addEdge(uint64_t id1, uint64_t id2);
  int removeNode(uint64_t id);
  int removeEdge(uint64_t id1, uint64_t id2);
  bool getNode(uint64_t id);
  int getEdge(uint64_t id1, uint64_t id2);
  IdSet *getNeighbors(uint64_t id);
  int shortestPath(uint64_t id1, uint64_t id2);
  bool checkpoint();

  void print();

private:
  bool idExists(uint64_t id);
  bool idInSet(IdSet *idSet, uint64_t id);
  IdSet *neighborList(uint64_t id);
  bool edgeExists(uint64_t id1, uint64_t id2);

  // The array starts with the number of nodes.
  // The nodes are converted into an array uint64_t, where each <key, value>
  // becomes [key, n, value1, value2, value3, ..., valuen].
  const uint64_t *loadToArray(size_t& size);
  void loadFromArray(const uint64_t *data, size_t size);

  // init() helpers
  void loadCheckpoint();
  void playbackLog();

  bool doNotLog = false;
  NodeList nodes;
};