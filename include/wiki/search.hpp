#pragma once

#include <functional>
#include <string>
#include <vector>

#include "wiki/graph.hpp"

namespace wiki
{
    using NeighbourProvider = std::function<std::vector<std::string>(const std::string &)>;
    using FrontierScorer = std::function<double(const std::string &node, const std::string &goal)>;

    std::vector<std::string> bfs(const std::string &start, const std::string &target, const NeighbourProvider &getNeighbours);

    std::vector<std::string> bidirectionalBfs(const std::string &start, const std::string &target, const NeighbourProvider &getNeighbours, const NeighbourProvider &getReverseNeighbours, const FrontierScorer &scoreFrontier = {});
}
