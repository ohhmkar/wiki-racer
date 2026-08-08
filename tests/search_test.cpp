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

  auto path = wiki::bfs("A", "D", [&](const std::string &node)
                        { return graph.neighbours(node); });

  EXPECT_EQ(path.size(), 3);
  EXPECT_EQ(path.front(), "A");
  EXPECT_EQ(path.back(), "D");
}

TEST(BFS, ReturnsEmptyWhenNoPathExists)
{
  wiki::Graph graph;

  graph.addEdge("A", "B");
  graph.addEdge("C", "D");

  auto path = wiki::bfs("A", "D", [&](const std::string &node)
                        { return graph.neighbours(node); });

  EXPECT_TRUE(path.empty());
}

TEST(BFS, StartEqualsTarget)
{
  wiki::Graph graph;

  auto path = wiki::bfs("A", "A", [&](const std::string &node)
                        { return graph.neighbours(node); });

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

  auto path = wiki::bfs("A", "E", [&](const std::string &node)
                        { return graph.neighbours(node); });

  std::vector<std::string> expected = {"A", "E"};

  EXPECT_EQ(path, expected);
}

TEST(Graph, FindsReverseNeighbours)
{
  wiki::Graph graph;

  graph.addEdge("A", "B");
  graph.addEdge("C", "B");
  graph.addEdge("B", "D");

  auto neighbours = graph.reverseNeighbours("B");

  std::sort(neighbours.begin(), neighbours.end());

  std::vector<std::string> expected = {"A", "C"};
  EXPECT_EQ(
      neighbours, expected);
}

TEST(BidirectionalBFS, FindsShortestPath)
{
  wiki::Graph graph;

  graph.addEdge("A", "B");
  graph.addEdge("B", "C");
  graph.addEdge("C", "D");

  auto path = wiki::bidirectionalBfs(
      "A", "D",
      [&](const std::string &node)
      { return graph.neighbours(node); },
      [&](const std::string &node)
      { return graph.reverseNeighbours(node); });

  std::vector<std::string> expected = {"A", "B", "C", "D"};
  EXPECT_EQ(path, expected);
}
TEST(BidirectionalBFS, ReturnsEmptyWhenNoPathExists)
{
  wiki::Graph graph;

  graph.addEdge("A", "B");
  graph.addEdge("C", "D");

  auto path = wiki::bidirectionalBfs(
      "A", "D",
      [&](const std::string &node)
      { return graph.neighbours(node); },
      [&](const std::string &node)
      { return graph.reverseNeighbours(node); });

  EXPECT_TRUE(path.empty());
}
TEST(BidirectionalBFS, StartEqualsTarget)
{
  wiki::Graph graph;

  auto path = wiki::bidirectionalBfs(
      "A", "A",
      [&](const std::string &node)
      { return graph.neighbours(node); },
      [&](const std::string &node)
      { return graph.reverseNeighbours(node); });

  std::vector<std::string> expected = {"A"};
  EXPECT_EQ(path, expected);
}
TEST(BidirectionalBFS, FindsShortestPathRatherThanFirstFoundPath)
{
  wiki::Graph graph;

  graph.addEdge("A", "B");
  graph.addEdge("B", "C");
  graph.addEdge("C", "D");
  graph.addEdge("D", "E");
  graph.addEdge("A", "E");

  auto path = wiki::bidirectionalBfs(
      "A", "E",
      [&](const std::string &node)
      { return graph.neighbours(node); },
      [&](const std::string &node)
      { return graph.reverseNeighbours(node); });

  std::vector<std::string> expected = {"A", "E"};
  EXPECT_EQ(path, expected);
}

TEST(BidirectionalBFS, MeetingDetectedDuringBackwardExpansion)
{
  wiki::Graph graph;

  graph.addEdge("S", "A");
  graph.addEdge("A", "B");
  graph.addEdge("B", "C");
  graph.addEdge("C", "T");

  auto path = wiki::bidirectionalBfs(
      "S", "T",
      [&](const std::string &node)
      { return graph.neighbours(node); },
      [&](const std::string &node)
      { return graph.reverseNeighbours(node); });

  std::vector<std::string> expected = {"S", "A", "B", "C", "T"};
  EXPECT_EQ(path, expected);
}
