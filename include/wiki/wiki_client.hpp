#pragma once

#include <string>
#include <vector>
#include <unordered_map>

namespace wiki
{
    class WikiClient
    {
    private:
        mutable std::unordered_map<std::string, std::vector<std::string>> cache;
        mutable std::unordered_map<std::string, std::vector<std::string>> backlinkCache;

    public:
        std::vector<std::string> getLinks(const std::string &title) const;
        std::vector<std::string> getBacklinks(const std::string &title) const;
    };
}
