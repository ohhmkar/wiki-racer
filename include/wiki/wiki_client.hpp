#pragma once

#include <string>
#include <vector>

namespace wiki
{
    class WikiClient
    {
    public:
        std::vector<std::string> getLinks(const std::string &title) const;
    };
}
