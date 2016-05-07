#include "Graph.h"

bool Graph::idExists(unsigned int id) {
  return nodes.find(id) != nodes.end();
}

bool Graph::idInSet(IdSet *idSet, unsigned int id) {
  return idSet->find(id) != idSet->end();
}

Graph::IdSet* Graph::neighborList(unsigned int id) {
  return nodes.find(id)->second;
}

bool Graph::edgeExists(unsigned int id1, unsigned int id2) {
  if (idInSet(neighborList(id1), id2)) return true;
  return false;
}

int Graph::addNode(unsigned int id) {
  // Make sure id doesn't exist already.
  if (idExists(id)) return -1;

  // Add the node to graph.
  nodes[id] = new IdSet();

  return 0;
}

int Graph::addEdge(unsigned int id1, unsigned int id2) {
  // Make sure id1 != id2.
  if (id1 == id2) return -2;

  // Make sure both nodes exist.
  // if (!idExists(id1) || !idExists(id2)) return -2;

  // Make sure id1 exists.
  if (!idExists(id1)) return -2;

  // Make sure edge does not already exist.
  if (edgeExists(id1, id2)) return -1;
  // assert(!edgeExists(id2, id1));

  // Add edge.
  neighborList(id1)->insert(id2);

  return 0;
}

int Graph::removeEdge(unsigned int id1, unsigned int id2) {
  // Make sure both nodes exist.
  // if (!idExists(id1) || !idExists(id2)) return -2;
  if (!idExists(id1)) return -2;

  IdSet *neighbors1 = neighborList(id1);

  // Make sure edge exists.
  if (!idInSet(neighbors1, id2)) return -2;

  // Remove edge.
  neighbors1->erase(id2);

  return 0;
}

int Graph::getNode(unsigned int id) {
  return idExists(id);
}

int Graph::getEdge(unsigned int id1, unsigned int id2) {
  // Make sure both nodes exist.
  // if (!idExists(id1) || !idExists(id2)) return -2;
  if (!idExists(id1)) return -2;

  return edgeExists(id1, id2);
}

Graph::IdSet *Graph::getNeighbors(unsigned int id) {
  // Make sure id exists.
  if (!idExists(id)) return NULL;

  return neighborList(id);
}

void Graph::print() {
  printf("Current graph state:\n");
  for (const auto& node: nodes) {
    unsigned int id = node.first;
    IdSet *neighbors = node.second;

    printf("%u: ", id);
    for (unsigned int neighbor: *neighbors) {
      printf("%u ", neighbor);
    }
    printf("\n");
  }
}