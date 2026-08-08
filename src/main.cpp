#include <iostream>

#include "wiki/search.hpp"
#include "wiki/wiki_client.hpp"

int main()
{
  wiki::WikiClient wiki;

  auto path = wiki::bfs("Cat", "Dog", [&](const std::string &node)
                        { return wiki.getLinks(node); });

  for (const auto &page : path)
  {
    std::cout << page << "\n";
  }
  return 0;
}
