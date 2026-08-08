#include <iostream>

#include "wiki/graph.hpp"

int main() {
  wiki::Graph graph;

  graph.addEdge("A", "B");
  graph.addEdge("B", "C");
  graph.addEdge("B", "D");
  graph.addEdge("A", "C");

  for (const auto &node : graph.neighbours("A")) {
    std::cout << node << "\n";
  }
}
