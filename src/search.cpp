#include "wiki/search.hpp"

#include <algorithm>
#include <queue>
#include <unordered_map>
#include <unordered_set>

namespace wiki
{

  std::vector<std::string> bfs(const Graph &graph, const std::string &start, const std::string &target)
  {

    if (start == target)
      return {start};
    std::queue<std::string> queue;
    std::unordered_set<std::string> visited;
    std::unordered_map<std::string, std::string> parent;

    queue.push(start);
    visited.insert(start);

    while (!queue.empty())
    {
      std::string current = queue.front();
      queue.pop();

      if (current == target)
      {
        break;
      }

      for (const auto &neighbour : graph.neighbours(current))
      {
        if (visited.contains(neighbour))
        {
          continue;
        }
        visited.insert(neighbour);
        parent[neighbour] = current;
        queue.push(neighbour);
      }
    }

    if (!visited.contains(target))
    {
      return {};
    }

    std::vector<std::string> path;
    std::string current = target;

    while (current != start)
    {
      path.push_back(current);
      current = parent[current];
    }

    path.push_back(start);

    std::reverse(path.begin(), path.end());

    return path;
  }

} // namespace wiki
