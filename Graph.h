#include <climits>
#include <unordered_set>
#include <unordered_map>

class Graph {
  typedef std::unordered_set<unsigned int> IdSet;
  typedef std::unordered_map<unsigned int, IdSet> NodeList;

public:
  int addNode(unsigned int id);
  int addEdge(unsigned int id1, unsigned int id2);
  int removeNode(unsigned int id);
  int removeEdge(unsigned int id1, unsigned int id2);
  int getNode(unsigned int id);
  int getEdge(unsigned int id1, unsigned int id2);
  IdSet *getNeighbors(unsigned int id);
  int shortestPath(unsigned int id1, unsigned int id2);

private:
  bool idExists(unsigned int id);
  bool idInSet(IdSet *idSet, unsigned int id);
  IdSet *neighborList(unsigned int id);
  bool edgeExists(unsigned int id1, unsigned int id2);

  NodeList nodes;
};