#pragma once

#include <string>
#include <unordered_map>
#include <vector>

namespace wiki {

class Graph {
private:
  std::unordered_map<std::string, std::vector<std::string>> adjList;

public:
  void addEdge(const std::string &from, const std::string &to);

  std::vector<std::string> neighbours(const std::string &node) const;
};

} // namespace wiki
