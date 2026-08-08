#include <gtest/gtest.h>

#include "wiki/graph.hpp"
#include "wiki/search.hpp"

TEST(BFS, FindsShortestPath)
{
  wiki::Graph graph;

  graph.addEdge("A", "B");
  graph.addEdge("A", "C");
  graph.addEdge("B", "D");
  graph.addEdge("C", "D");

  auto path = wiki::bfs(graph, "A", "D");

  EXPECT_EQ(path.size(), 3);
  EXPECT_EQ(path.front(), "A");
  EXPECT_EQ(path.back(), "D");
}

TEST(BFS, ReturnsEmptyWhenNoPathExists)
{
  wiki::Graph graph;

  graph.addEdge("A", "B");
  graph.addEdge("C", "D");

  auto path = wiki::bfs(graph, "A", "D");

  EXPECT_TRUE(path.empty());
}

TEST(BFS, StartEqualsTarget)
{
  wiki::Graph graph;

  auto path = wiki::bfs(graph, "A", "A");

  std::vector<std::string> expected = {"A"};

  EXPECT_EQ(path, expected);
}

TEST(BFS, FindsShortestPathRatherThanFirstFoundPath)
{

  wiki::Graph graph;

  graph.addEdge("A", "B");
  graph.addEdge("B", "C");
  graph.addEdge("C", "D");
  graph.addEdge("D", "E");
  graph.addEdge("A", "E");

  auto path = wiki::bfs(graph, "A", "E");

  std::vector<std::string> expected = {"A", "E"};

  EXPECT_EQ(path, expected);
}
