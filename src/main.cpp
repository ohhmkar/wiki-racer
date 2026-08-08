#include <iostream>

#include "wiki/wiki_client.hpp"

int main()
{
  wiki::WikiClient wiki;

  auto links = wiki.getLinks("Cat");

  for (const auto &link : links)
  {
    std::cout << link << '\n';
  }

  return 0;
}
