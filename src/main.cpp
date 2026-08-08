#include <iostream>

#include "wiki/graph.hpp"
#include "wiki/search.hpp"

int main() {
  wiki::Graph graph;

  graph.addEdge("A", "B");
  graph.addEdge("B", "C");
  graph.addEdge("B", "D");
  graph.addEdge("A", "C");

  auto path = wiki::bfs(graph, "A", "D");

  for (const auto &node : path) {
    std::cout << node;

    if (node != path.back()) {
      std::cout << " -> ";
    }
  }

  std::cout << "\n";
  return 0;
}
