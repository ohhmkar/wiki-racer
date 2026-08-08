#pragma once

#include <string>
#include <vector>

#include "wiki/graph.hpp"

namespace wiki {
std::vector<std::string> bfs(const Graph &graph, const std::string &start,
                             const std::string &target);
}
