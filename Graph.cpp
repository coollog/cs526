#include "Graph.h"

bool Graph::idExists(uint64_t id) {
  return nodes.find(id) != nodes.end();
}

bool Graph::idInSet(IdSet *idSet, uint64_t id) {
  return idSet->find(id) != idSet->end();
}

Graph::IdSet* Graph::neighborList(uint64_t id) {
  return &nodes.find(id)->second;
}

bool Graph::edgeExists(uint64_t id1, uint64_t id2) {
  if (idInSet(neighborList(id1), id2)) return true;
  return false;
}

int Graph::addNode(uint64_t id) {
  // Make sure id doesn't exist already.
  if (idExists(id)) return -1;

  // Add the node to graph.
  nodes[id] = IdSet();

  return 0;
}

int Graph::addEdge(uint64_t id1, uint64_t id2) {
  // Make sure id1 != id2.
  if (id1 == id2) return -2;

  // Make sure both nodes exist.
  if (!idExists(id1) || !idExists(id2)) return -2;

  // Make sure edge does not already exist.
  if (edgeExists(id1, id2)) return -1;
  assert(!edgeExists(id2, id1));

  // Add edge.
  neighborList(id1)->insert(id2);
  neighborList(id2)->insert(id1);

  return 0;
}

int Graph::removeNode(uint64_t id) {
  // Make sure node exists.
  if (!idExists(id)) return -2;

  // Remove all edges.
  IdSet *neighbors = neighborList(id);
  for (uint64_t neighborId: *neighbors) {
    removeEdge(id, neighborId);
  }

  // Remove node.
  nodes.erase(id);

  return 0;
}

int Graph::removeEdge(uint64_t id1, uint64_t id2) {
  // Make sure both nodes exist.
  if (!idExists(id1) || !idExists(id2)) return -2;

  IdSet *neighbors1 = neighborList(id1);
  IdSet *neighbors2 = neighborList(id2);

  // Make sure edge exists.
  if (!idInSet(neighbors1, id2)) return -2;
  assert(idInSet(neighbors2, id1));

  // Remove edge.
  neighbors1->erase(id2);
  neighbors2->erase(id1);

  return 0;
}

bool Graph::getNode(uint64_t id) {
  return idExists(id);
}

int Graph::getEdge(uint64_t id1, uint64_t id2) {
  // Make sure both nodes exist.
  if (!idExists(id1) || !idExists(id2)) return -2;

  assert(edgeExists(id1, id2) == edgeExists(id2, id1));
  return edgeExists(id1, id2);
}

Graph::IdSet *Graph::getNeighbors(uint64_t id) {
  // Make sure id exists.
  if (!idExists(id)) return NULL;

  return neighborList(id);
}


int Graph::shortestPath(uint64_t idSource, uint64_t idDest) {
  // Make sure both nodes exist.
  if (!idExists(idSource) || !idExists(idDest)) return -2;

  typedef std::unordered_map<uint64_t, int> NodeMap;

  IdSet nodeQueue;
  NodeMap dist, prev;

  // Set up.
  for (const auto& node: nodes) {
    uint64_t id = node.first;

    dist[id] = INT_MAX;
    prev[id] = -1;

    nodeQueue.insert(id);
  }

  dist[idSource] = 0;

  while (nodeQueue.size() > 0) {
    // Get the min dist node.
    int minDist = INT_MAX;
    int minDistId = -1;

    for (uint64_t id: nodeQueue) {
      if (minDistId == -1 || dist[id] < minDist) {
        minDistId = id;
        minDist = dist[id];
      }
    }

    if (minDistId == (int)idDest) {
      return dist[idDest];
    }

    // Remove it from the queue.
    nodeQueue.erase(minDistId);

    // Process the neighbors.
    for (uint64_t id: *getNeighbors(minDistId)) {
      int alt = dist[minDistId] + 1;
      if (alt < dist[id]) {
        dist[id] = alt;
        prev[id] = minDistId;
      }
    }
  }

  return -1;
}

bool Graph::checkpoint() {
  // TODO: Add implementation.

  return true;
}