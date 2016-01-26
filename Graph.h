#include <vector>
#include <unordered_map>

class Graph {
  typedef std::vector<unsigned int> id_list;
  typedef std::unordered_map<unsigned int, id_list> node_list;

public:
  int addNode(unsigned int id);
  int addEdge(unsigned int id1, unsigned int id2);

private:
  node_list nodes;
};