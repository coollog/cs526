#include "Graph.h"

bool Graph::idExists(unsigned int id) {
  return nodes.find(id) != nodes.end();
}

bool Graph::idInSet(IdSet *idSet, unsigned int id) {
  return idSet->find(id) != idSet->end();
}

Graph::IdSet* Graph::neighborList(unsigned int id) {
  return &nodes.find(id)->second;
}

bool Graph::edgeExists(unsigned int id1, unsigned int id2) {
  if (idInSet(neighborList(id1), id2)) return true;
  return false;
}

int Graph::addNode(unsigned int id) {
  // Make sure id doesn't exist already.
  if (idExists(id)) return -1;

  // Add the node to graph.
  nodes[id] = IdSet();

  return 0;
}

int Graph::addEdge(unsigned int id1, unsigned int id2) {
  // Make sure id1 != id2.
  if (id1 == id2) return -2;

  // Make sure both nodes exist.
  if (!idExists(id1) || !idExists(id2)) return -2;

  // Make sure edge does not already exist.
  if (edgeExists(id1, id2)) return -1;

  // Add edge.
  neighborList(id1)->insert(id2);
  neighborList(id2)->insert(id1);

  return 0;
}

int Graph::removeNode(unsigned int id) {
  // Make sure node exists.
  if (!idExists(id)) return -2;

  // Remove all edges.
  IdSet *neighbors = neighborList(id);
  for (unsigned int neighborId: *neighbors) {
    removeEdge(id, neighborId);
  }

  // Remove node.
  nodes.erase(id);

  return 0;
}

int Graph::removeEdge(unsigned int id1, unsigned int id2) {
  IdSet *neighbors1 = neighborList(id1);
  IdSet *neighbors2 = neighborList(id2);

  // Make sure edge exists.
  if (idInSet(neighbors1, id2)) return -2;

  // Remove edge.
  neighbors1->erase(id2);
  neighbors2->erase(id1);

  return 0;
}

int Graph::getNode(unsigned int id) {
  return idExists(id);
}

int Graph::getEdge(unsigned int id1, unsigned int id2) {
  // Make sure both nodes exist.
  if (!idExists(id1) || !idExists(id2)) return -2;

  return edgeExists(id1, id2);
}

Graph::IdSet *Graph::getNeighbors(unsigned int id) {
  // Make sure id exists.
  if (!idExists(id)) return NULL;

  return neighborList(id);
}


int Graph::shortestPath(unsigned int idSource, unsigned int idDest) {
  // Make sure both nodes exist.
  if (!idExists(idSource) || !idExists(idDest)) return -2;

  typedef std::unordered_map<unsigned int, int> NodeMap;

  IdSet nodeQueue;
  NodeMap dist, prev;

  // Set up.
  for (const auto& node: nodes) {
    unsigned int id = node.first;

    dist[id] = INT_MAX;
    prev[id] = -1;

    nodeQueue.insert(id);
  }

  dist[idSource] = 0;

  while (nodeQueue.size() > 0) {
    // Get the min dist node.
    int minDist = INT_MAX;
    unsigned int minDistId = -1;

    for (unsigned int id: nodeQueue) {
      if (minDistId == -1 || dist[id] < minDist) {
        minDistId = id;
        minDist = dist[id];
      }
    }

    if (minDistId == idDest) {
      return dist[idDest];
    }

    // Remove it from the queue.
    nodeQueue.erase(minDistId);

    // Process the neighbors.
    for (unsigned int id: *getNeighbors(minDistId)) {
      int alt = dist[minDistId] + 1;
      if (alt < dist[id]) {
        dist[id] = alt;
        prev[id] = minDistId;
      }
    }
  }

  return -1;
}