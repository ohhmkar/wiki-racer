#include "wiki/search.hpp"

#include <algorithm>
#include <atomic>
#include <exception>
#include <iostream>
#include <mutex>
#include <queue>
#include <thread>
#include <unordered_map>
#include <unordered_set>
#include <utility>

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
    std::vector<std::pair<std::string, std::vector<std::string>>> fetchNeighboursConcurrently(
        const std::vector<std::string> &nodes,
        const NeighbourProvider &getNeighbours)
    {
      std::vector<std::pair<std::string, std::vector<std::string>>> results(nodes.size());

      if (nodes.empty())
        return results;

      std::size_t numThreads = std::thread::hardware_concurrency();
      if (numThreads == 0)
        numThreads = 1;
      numThreads = std::min(numThreads, nodes.size());

      std::atomic<std::size_t> next{0};
      std::mutex errorMutex;
      std::exception_ptr error;

      std::vector<std::thread> workers;
      workers.reserve(numThreads);

      for (std::size_t i = 0; i < numThreads; ++i)
      {
        workers.emplace_back([&] {
          while (true)
          {
            std::size_t idx = next.fetch_add(1);
            if (idx >= nodes.size())
              break;

            try
            {
              results[idx] = {nodes[idx], getNeighbours(nodes[idx])};
            }
            catch (...)
            {
              std::lock_guard<std::mutex> lock(errorMutex);
              if (!error)
                error = std::current_exception();
              break;
            }
          }
        });
      }

      for (auto &worker : workers)
        worker.join();

      if (error)
        std::rethrow_exception(error);

      return results;
    }

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
      constexpr std::size_t batchSize = 4;

      std::vector<std::string> forwardLevel;
      std::size_t forwardSize = forwardQueue.size();

      while (forwardSize--)
      {
        forwardLevel.push_back(forwardQueue.front());
        forwardQueue.pop();
      }

      while (!forwardLevel.empty())
      {
        std::size_t batch = std::min(batchSize, forwardLevel.size());
        std::vector<std::string> batchNodes(forwardLevel.begin(), forwardLevel.begin() + batch);
        forwardLevel.erase(forwardLevel.begin(), forwardLevel.begin() + batch);

        auto results = fetchNeighboursConcurrently(batchNodes, getNeighbours);

        for (const auto &[current, neighbours] : results)
        {
          for (const auto &neighbour : neighbours)
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
      }

      std::vector<std::string> backwardLevel;
      std::size_t backwardSize = backwardQueue.size();

      while (backwardSize--)
      {
        backwardLevel.push_back(backwardQueue.front());
        backwardQueue.pop();
      }

      while (!backwardLevel.empty())
      {
        std::size_t batch = std::min(batchSize, backwardLevel.size());
        std::vector<std::string> batchNodes(backwardLevel.begin(), backwardLevel.begin() + batch);
        backwardLevel.erase(backwardLevel.begin(), backwardLevel.begin() + batch);

        auto results = fetchNeighboursConcurrently(batchNodes, getReverseNeighbours);

        for (const auto &[current, neighbours] : results)
        {
          for (const auto &neighbour : neighbours)
          {
            if (backwardParent.contains(neighbour))
            {
              continue;
            }

            backwardParent[neighbour] = current;
            backwardQueue.push(neighbour);

            if (forwardParent.contains(neighbour))
            {
              return reconstructPath(neighbour, forwardParent, backwardParent);
            }
          }
        }
      }
    }

    return {};
  }

} // namespace wiki
