#include "wiki/graph.hpp"
#include "wiki/search.hpp"

#include <chrono>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <random>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

namespace
{
  using Clock = std::chrono::steady_clock;

  struct RunResult
  {
    long long fetches = 0;
    std::size_t pathLength = 0;
    double seconds = 0.0;
  };

  template <typename Search>
  RunResult measure(Search search)
  {
    std::ostringstream quiet;
    auto *old = std::cout.rdbuf(quiet.rdbuf());

    RunResult r;
    auto start = Clock::now();
    auto path = search();
    auto end = Clock::now();

    std::cout.rdbuf(old);

    r.pathLength = path.size();
    r.seconds = std::chrono::duration<double>(end - start).count();
    return r;
  }

  struct GeneratedGraph
  {
    wiki::Graph graph;
    std::unordered_map<std::string, std::vector<std::string>> reverse;
  };

  GeneratedGraph randomGraph(std::size_t nodes, std::size_t outDegree, unsigned seed)
  {
    std::mt19937 rng(seed);
    std::uniform_int_distribution<std::size_t> dist(0, nodes - 1);

    GeneratedGraph g;
    for (std::size_t i = 0; i < nodes; ++i)
    {
      std::string from = "n" + std::to_string(i);
      for (std::size_t j = 0; j < outDegree; ++j)
      {
        std::size_t to = dist(rng);
        if (to == i)
          to = (to + 1) % nodes;

        std::string target = "n" + std::to_string(to);
        g.graph.addEdge(from, target);
        g.reverse[target].push_back(from);
      }
    }
    return g;
  }

  GeneratedGraph gridGraph(std::size_t rows, std::size_t cols)
  {
    auto label = [](std::size_t r, std::size_t c)
    { return std::to_string(r) + "," + std::to_string(c); };

    GeneratedGraph g;
    for (std::size_t r = 0; r < rows; ++r)
    {
      for (std::size_t c = 0; c < cols; ++c)
      {
        if (r + 1 < rows)
        {
          auto a = label(r, c);
          auto b = label(r + 1, c);
          g.graph.addEdge(a, b);
          g.graph.addEdge(b, a);
          g.reverse[b].push_back(a);
          g.reverse[a].push_back(b);
        }
        if (c + 1 < cols)
        {
          auto a = label(r, c);
          auto b = label(r, c + 1);
          g.graph.addEdge(a, b);
          g.graph.addEdge(b, a);
          g.reverse[b].push_back(a);
          g.reverse[a].push_back(b);
        }
      }
    }
    return g;
  }

  double manhattan(const std::string &a, const std::string &b)
  {
    auto parse = [](const std::string &s)
    {
      auto comma = s.find(',');
      return std::make_pair(std::stoi(s.substr(0, comma)), std::stoi(s.substr(comma + 1)));
    };

    auto [ar, ac] = parse(a);
    auto [br, bc] = parse(b);
    return static_cast<double>(std::abs(ar - br) + std::abs(ac - bc));
  }

  void printHeader()
  {
    std::cout << std::left
              << std::setw(48) << "benchmark"
              << std::setw(12) << "fetches"
              << std::setw(10) << "path len"
              << std::setw(12) << "time (s)"
              << "\n"
              << std::string(84, '-') << "\n";
  }

  void printRow(const std::string &name, const RunResult &r)
  {
    std::cout << std::left
              << std::setw(48) << name
              << std::setw(12) << r.fetches
              << std::setw(10) << r.pathLength
              << std::setw(12) << std::fixed << std::setprecision(3) << r.seconds
              << "\n";
  }

  void benchRandomGraph()
  {
    std::cout << "\n== Random directed graph, out-degree 25 (Wikipedia-like density) ==\n";
    printHeader();

    for (std::size_t nodes : {20000ul, 60000ul, 120000ul})
    {
      auto g = randomGraph(nodes, 25, 42);
      std::string start = "n0";
      std::string target = "n" + std::to_string(nodes - 1);

      long long fetches = 0;
      auto res = measure([&]
                         {
                           return wiki::bfs(start, target,
                                            [&](const std::string &node)
                                            {
                                              ++fetches;
                                              return g.graph.neighbours(node);
                                            });
                         });
      res.fetches = fetches;
      printRow("bfs N=" + std::to_string(nodes), res);

      fetches = 0;
      res = measure([&]
                    {
                      return wiki::bidirectionalBfs(
                          start, target,
                          [&](const std::string &node)
                          {
                            ++fetches;
                            return g.graph.neighbours(node);
                          },
                          [&](const std::string &node)
                          {
                            ++fetches;
                            return g.reverse[node];
                          });
                    });
      res.fetches = fetches;
      printRow("bidirectionalBfs N=" + std::to_string(nodes), res);
    }
  }

  void benchGrid()
  {
    std::cout << "\n== 250x250 undirected grid (62,500 nodes), corner to corner ==\n";
    printHeader();

    auto g = gridGraph(250, 250);
    std::string start = "0,0";
    std::string target = "249,249";
    const std::size_t expectedPath = 499;

    long long fetches = 0;
    auto res = measure([&]
                       {
                         return wiki::bfs(start, target,
                                          [&](const std::string &node)
                                          {
                                            ++fetches;
                                            return g.graph.neighbours(node);
                                          });
                       });
    res.fetches = fetches;
    printRow("bfs (no heuristic)", res);
    if (res.pathLength != expectedPath)
      std::cout << "  !!! expected path length " << expectedPath << "\n";

    fetches = 0;
    res = measure([&]
                  {
                    return wiki::bidirectionalBfs(
                        start, target,
                        [&](const std::string &node)
                        {
                          ++fetches;
                          return g.graph.neighbours(node);
                        },
                        [&](const std::string &node)
                        {
                          ++fetches;
                          return g.reverse[node];
                        });
                  });
    res.fetches = fetches;
    printRow("bidirectionalBfs (no scorer)", res);
    if (res.pathLength != expectedPath)
      std::cout << "  !!! expected path length " << expectedPath << "\n";

    fetches = 0;
    res = measure([&]
                  {
                    return wiki::bidirectionalBfs(
                        start, target,
                        [&](const std::string &node)
                        {
                          ++fetches;
                          return g.graph.neighbours(node);
                        },
                        [&](const std::string &node)
                        {
                          ++fetches;
                          return g.reverse[node];
                        },
                        [&](const std::string &node, const std::string &goal)
                        { return -manhattan(node, goal); });
                  });
    res.fetches = fetches;
    printRow("bidirectionalBfs (manhattan scorer)", res);
    if (res.pathLength != expectedPath)
      std::cout << "  !!! expected path length " << expectedPath << "\n";

    fetches = 0;
    res = measure([&]
                  {
                    return wiki::bidirectionalBfs(
                        start, target,
                        [&](const std::string &node)
                        {
                          ++fetches;
                          return g.graph.neighbours(node);
                        },
                        [&](const std::string &node)
                        {
                          ++fetches;
                          return g.reverse[node];
                        },
                        [&](const std::string &node, const std::string &goal)
                        { return manhattan(node, goal); });
                  });
    res.fetches = fetches;
    printRow("bidirectionalBfs (pessimistic scorer)", res);
    if (res.pathLength != expectedPath)
      std::cout << "  !!! expected path length " << expectedPath << "\n";
  }

  void benchHeuristic()
  {
    std::cout << "\n== Spoke graph: 1,000 start-spokes, 20 topic-matched, 1 bridges to target ==\n";
    printHeader();

    const int spokes = 1000;
    const int matched = 20;
    const int bridge = 17;

    wiki::Graph graph;
    std::unordered_map<std::string, std::vector<std::string>> reverse;

    std::vector<std::string> order;
    for (int i = 0; i < spokes; ++i)
      order.push_back("a_" + std::to_string(i));
    std::shuffle(order.begin(), order.end(), std::mt19937(42));

    for (const auto &spoke : order)
    {
      graph.addEdge("S", spoke);
      reverse[spoke].push_back("S");
    }

    std::string bridgeNode = "a_" + std::to_string(bridge);
    graph.addEdge(bridgeNode, "b_1");
    reverse["b_1"].push_back(bridgeNode);

    for (int i = 0; i < spokes; ++i)
    {
      graph.addEdge("b_" + std::to_string(i), "T");
      reverse["T"].push_back("b_" + std::to_string(i));
    }

    auto isMatched = [&](const std::string &node)
    {
      if (node.rfind("a_", 0) != 0)
        return false;
      int idx = std::stoi(node.substr(2));
      return idx < matched;
    };

    long long fetches = 0;
    auto res = measure([&]
                       {
                         return wiki::bidirectionalBfs(
                             "S", "T",
                             [&](const std::string &node)
                             {
                               ++fetches;
                               return graph.neighbours(node);
                             },
                             [&](const std::string &node)
                             {
                               ++fetches;
                               return reverse[node];
                             });
                       });
    res.fetches = fetches;
    printRow("no scorer", res);

    fetches = 0;
    res = measure([&]
                  {
                    return wiki::bidirectionalBfs(
                        "S", "T",
                        [&](const std::string &node)
                        {
                          ++fetches;
                          return graph.neighbours(node);
                        },
                        [&](const std::string &node)
                        {
                          ++fetches;
                          return reverse[node];
                        },
                        [&](const std::string &node, const std::string &)
                        { return isMatched(node) ? 1.0 : 0.0; });
                  });
    res.fetches = fetches;
    printRow("topic-aware scorer", res);
  }

  void benchReverseNeighbours()
  {
    std::cout << "\n== Graph::reverseNeighbours (naive O(V+E) scan per call) ==\n";
    printHeader();

    auto g = randomGraph(10000, 20, 7);

    long long calls = 0;
    auto res = measure([&]
                       {
                         long long work = 0;
                         for (int i = 0; i < 100; ++i)
                         {
                           ++calls;
                           auto out = g.graph.reverseNeighbours("n" + std::to_string(1000 + i));
                           work += static_cast<long long>(out.size());
                         }
                         return std::vector<std::string>(static_cast<std::size_t>(work), "x");
                       });
    res.fetches = calls;
    res.pathLength = 0;
    printRow("100 reverseNeighbours calls (10k nodes, 20 deg)", res);
  }
}

int main()
{
  std::cout << "wiki-racer benchmarks\n"
            << "========================================\n";

  benchRandomGraph();
  benchGrid();
  benchHeuristic();
  benchReverseNeighbours();

  std::cout << "\n";
  return 0;
}
