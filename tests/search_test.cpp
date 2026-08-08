#include <gtest/gtest.h>

#include "wiki/graph.hpp"
#include "wiki/search.hpp"

TEST(BFS, FindsShortestPath) {
  wiki::Graph graph;

  graph.addEdge("A", "B");
  graph.addEdge("A", "C");
  graph.addEdge("B", "D");
  graph.addEdge("C", "D");

  auto path = wiki::bfs(graph, "A", "D");

  std::vector<std::string> expected = { "A", "B", "D";
}

EXPECT_EQ(path, expected);
}
