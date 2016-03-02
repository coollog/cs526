#include <cstdint>
#include <climits>
#include <unordered_set>
#include <unordered_map>
#include <assert.h>

class Graph {
public:
  typedef std::unordered_set<uint64_t> IdSet;
  typedef std::unordered_map<uint64_t, IdSet> NodeList;

  int addNode(uint64_t id);
  int addEdge(uint64_t id1, uint64_t id2);
  int removeNode(uint64_t id);
  int removeEdge(uint64_t id1, uint64_t id2);
  bool getNode(uint64_t id);
  int getEdge(uint64_t id1, uint64_t id2);
  IdSet *getNeighbors(uint64_t id);
  int shortestPath(uint64_t id1, uint64_t id2);
  bool checkpoint();

private:
  bool idExists(uint64_t id);
  bool idInSet(IdSet *idSet, uint64_t id);
  IdSet *neighborList(uint64_t id);
  bool edgeExists(uint64_t id1, uint64_t id2);

  NodeList nodes;
};