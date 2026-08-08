#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <mutex>

namespace wiki
{
    class WikiClient
    {
    private:
        mutable std::unordered_map<std::string, std::vector<std::string>> cache;
        mutable std::unordered_map<std::string, std::vector<std::string>> backlinkCache;
        mutable std::unordered_map<std::string, std::string> redirectCache;
        mutable std::mutex cacheMutex;

        std::string fetchJson(const std::string &url) const;
        std::string resolveTitle(const std::string &title) const;

    public:
        std::vector<std::string> getLinks(const std::string &title) const;
        std::vector<std::string> getBacklinks(const std::string &title) const;
    };
}
