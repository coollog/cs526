#include <climits>
#include <unordered_set>
#include <unordered_map>
#include <assert.h>

class Graph {
public:
  typedef std::unordered_set<unsigned int> IdSet;
  typedef std::unordered_map<unsigned int, IdSet*> NodeList;

  int addNode(unsigned int id);
  int addEdge(unsigned int id1, unsigned int id2);
  int removeEdge(unsigned int id1, unsigned int id2);
  int getNode(unsigned int id);
  int getEdge(unsigned int id1, unsigned int id2);
  IdSet *getNeighbors(unsigned int id);

  void print();

private:
  bool idExists(unsigned int id);
  bool idInSet(IdSet *idSet, unsigned int id);
  IdSet *neighborList(unsigned int id);
  bool edgeExists(unsigned int id1, unsigned int id2);

  NodeList nodes;
};