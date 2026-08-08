#include "wiki/search.hpp"

#include <algorithm>
#include <iostream>
#include <queue>
#include <unordered_map>
#include <unordered_set>

namespace wiki
{

  std::vector<std::string> bfs(const std::string &start, const std::string &target, const NeighbourProvider &getNeighbours)
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

      std::cout << "Visiting: " << current << "\n";
      if (current == target)
      {
        break;
      }

      for (const auto &neighbour : getNeighbours(current))
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

  namespace
  {
    std::vector<std::string> reconstructPath(const std::string &meeting, const std::unordered_map<std::string, std::string> &forwardParent, const std::unordered_map<std::string, std::string> &backwardParent)
    {
      std::vector<std::string> path;

      std::string current = meeting;
      while (!current.empty())
      {
        path.push_back(current);
        current = forwardParent.at(current);
      }
      std::reverse(path.begin(), path.end());

      current = backwardParent.at(meeting);
      while (!current.empty())
      {
        path.push_back(current);
        current = backwardParent.at(current);
      }

      return path;
    }
  } // namespace

  std::vector<std::string> bidirectionalBfs(const std::string &start, const std::string &target, const NeighbourProvider &getNeighbours, const NeighbourProvider &getReverseNeighbours)
  {

    if (start == target)
      return {start};

    std::queue<std::string> forwardQueue;
    std::queue<std::string> backwardQueue;

    std::unordered_map<std::string, std::string> forwardParent;
    std::unordered_map<std::string, std::string> backwardParent;

    forwardQueue.push(start);
    backwardQueue.push(target);

    forwardParent[start] = "";
    backwardParent[target] = "";

    while (!forwardQueue.empty() && !backwardQueue.empty())
    {
      std::size_t forwardSize = forwardQueue.size();

      while (forwardSize--)
      {
        std::string current = forwardQueue.front();
        forwardQueue.pop();

        for (const auto &neighbour : getNeighbours(current))
        {
          if (forwardParent.contains(neighbour))
          {
            continue;
          }

          forwardParent[neighbour] = current;
          forwardQueue.push(neighbour);

          if (backwardParent.contains(neighbour))
          {
            return reconstructPath(neighbour, forwardParent, backwardParent);
          }
        }
      }

      std::size_t backwardSize = backwardQueue.size();

      while (backwardSize--)
      {
        std::string current = backwardQueue.front();
        backwardQueue.pop();

        for (const auto &neighbour : getReverseNeighbours(current))
        {
          if (backwardParent.contains(neighbour))
            continue;

          backwardParent[neighbour] = current;
          backwardQueue.push(neighbour);

          if (forwardParent.contains(neighbour))
          {
            return reconstructPath(neighbour, forwardParent, backwardParent);
          }
        }
      }
    }

    return {};
  }

} // namespace wiki
