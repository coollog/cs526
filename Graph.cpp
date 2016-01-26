#include "Graph.h"

int Graph::addNode(unsigned int id) {
  // Make sure id doesn't exist already.


  // Add the node to graph.
  nodes.insert({id, id_list()});

  return 0;
}

int Graph::addEdge(unsigned int id1, unsigned int id2) {

  return 0;
}