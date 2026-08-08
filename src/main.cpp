#include <iostream>

#include "wiki/search.hpp"
#include "wiki/wiki_client.hpp"

int main()
{
  wiki::WikiClient wiki;

  // auto path = wiki::bfs("Lebron James", "Spider-man", [&](const std::string &node)
  //                       { return wiki.getLinks(node); });
  /*
  auto backlinks = wiki.getBacklinks("Cat");
  for (const auto &page : backlinks)
    std::cout << page << "\n";
  */

  auto path = wiki::bidirectionalBfs(
      "Lebron James", "Spider-Man",
      [&](const std::string &node)
      { return wiki.getLinks(node); },
      [&](const std::string &node)
      { return wiki.getBacklinks(node); });

  for (const auto &page : path)
    std::cout << page << "\n";
  return 0;
}
