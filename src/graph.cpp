#include "wiki/graph.hpp"
#include <algorithm>

namespace wiki
{

  void Graph::addEdge(const std::string &from, const std::string &to)
  {
    adjList[from].push_back(to);
  }

  std::vector<std::string> Graph::neighbours(const std::string &node) const
  {
    auto it = adjList.find(node);

    if (it == adjList.end())
    {
      return {};
    }
    return it->second;
  }

  std::vector<std::string> Graph::reverseNeighbours(const std::string &node) const
  {
    std::vector<std::string> result;

    for (const auto &[from, neighbours] : adjList)
    {
      for (const auto &to : neighbours)
      {
        if (to == node)
        {
          result.push_back(from);
        }
      }
    }
    return result;
  }
} // namespace wiki
