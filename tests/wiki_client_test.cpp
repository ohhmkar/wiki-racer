#include <gtest/gtest.h>

#include "wiki/graph.hpp"
#include "wiki/search.hpp"

TEST(WikiClient, GetsLinks)
{
    wiki::WikiClient wiki;

    auto links = wiki.getLinks("Cat");

    EXPECT_FALSE(links.empty());
}
