#pragma once

#include <chrono>
#include <string>
#include <vector>
#include <unordered_map>
#include <mutex>
#include <atomic>

namespace wiki
{
    class WikiClient
    {
    private:
        mutable std::unordered_map<std::string, std::vector<std::string>> cache;
        mutable std::unordered_map<std::string, std::vector<std::string>> backlinkCache;
        mutable std::unordered_map<std::string, std::string> redirectCache;
        mutable std::mutex cacheMutex;
        mutable std::mutex throttleMutex;
        mutable std::chrono::steady_clock::time_point nextAllowedRequest;
        mutable std::atomic<long> httpRequests{0};

        void throttle() const;
        std::string fetchJson(const std::string &url) const;
        std::string resolveTitle(const std::string &title) const;

    public:
        long requestsMade() const { return httpRequests.load(); }
        std::vector<std::string> getLinks(const std::string &title) const;
        std::vector<std::string> getBacklinks(const std::string &title) const;
    };
}
