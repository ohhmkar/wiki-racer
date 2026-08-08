#include <cctype>
#include <exception>
#include <iostream>
#include <set>
#include <string>

#include "wiki/search.hpp"
#include "wiki/wiki_client.hpp"

namespace
{
  std::set<std::string> significantWords(const std::string &title)
  {
    const std::set<std::string> stopwords = {"the", "a", "an", "of", "and", "for", "in", "on", "at", "to", "is", "are"};

    std::set<std::string> words;
    std::string current;

    for (char c : title)
    {
      if (std::isalpha(static_cast<unsigned char>(c)))
      {
        current.push_back(static_cast<char>(std::tolower(static_cast<unsigned char>(c))));
      }
      else
      {
        if (!current.empty() && !stopwords.count(current))
          words.insert(current);
        current.clear();
      }
    }

    if (!current.empty() && !stopwords.count(current))
      words.insert(current);

    return words;
  }

  double titleSimilarity(const std::string &a, const std::string &b)
  {
    const auto wordsA = significantWords(a);
    const auto wordsB = significantWords(b);

    double shared = 0;
    for (const auto &word : wordsA)
    {
      if (wordsB.count(word))
        ++shared;
    }

    return shared;
  }
}

int main(int argc, char *argv[])
{
  const std::string start = argc > 1 ? argv[1] : "PhonePe";
  const std::string target = argc > 2 ? argv[2] : "Sardar Patel Institute of Technology";

  wiki::WikiClient wiki;

  try
  {
    auto path = wiki::bidirectionalBfs(
        start, target,
        [&](const std::string &node)
        { return wiki.getLinks(node); },
        [&](const std::string &node)
        { return wiki.getBacklinks(node); },
        titleSimilarity);

    for (const auto &page : path)
      std::cout << page << " -> ";

    std::cerr << "requests made: " << wiki.requestsMade() << "\n";
  }
  catch (const std::exception &e)
  {
    std::cerr << "search failed: " << e.what() << "\n";
    return 1;
  }

  return 0;
}
