#include "wiki/wiki_client.hpp"

#include <curl/curl.h>
#include <nlohmann/json.hpp>

#include <chrono>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>

namespace wiki
{

    namespace
    {
        size_t writeCallback(char *contents, size_t size, size_t nmemb, void *userp)
        {
            size_t totalSize = size * nmemb;

            auto *response = static_cast<std::string *>(userp);

            response->append(contents, totalSize);

            return totalSize;
        }

        std::string urlEncode(const std::string &text)
        {
            CURL *curl = curl_easy_init();
            if (!curl)
                throw std::runtime_error("curl init failed");

            char *encoded = curl_easy_escape(curl, text.c_str(), text.size());
            if (!encoded)
            {
                curl_easy_cleanup(curl);
                throw std::runtime_error("failed to encode title");
            }

            std::string result(encoded);
            curl_free(encoded);
            curl_easy_cleanup(curl);
            return result;
        }
    }

    std::string WikiClient::fetchJson(const std::string &url) const
    {
        constexpr int maxAttempts = 10;

        for (int attempt = 1; attempt <= maxAttempts; ++attempt)
        {
            CURL *curl = curl_easy_init();
            if (!curl)
                throw std::runtime_error("curl init failed");

            std::string response;

            curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
            curl_easy_setopt(curl, CURLOPT_USERAGENT, "wiki-racer/0.1 (https://github.com/ohhmkar/wiki-racer)");
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

            CURLcode result = curl_easy_perform(curl);

            long status = 0;
            curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &status);

            double retryAfter = 0;
            if (status == 429)
                curl_easy_getinfo(curl, CURLINFO_RETRY_AFTER, &retryAfter);

            curl_easy_cleanup(curl);

            if (result != CURLE_OK)
                throw std::runtime_error(curl_easy_strerror(result));

            if (status == 429 || status >= 500)
            {
                if (attempt >= maxAttempts)
                    throw std::runtime_error("API request failed with HTTP " + std::to_string(status));

                long waitMs = (retryAfter > 0)
                                  ? static_cast<long>(retryAfter * 1000)
                                  : 500 * attempt;

                std::this_thread::sleep_for(std::chrono::milliseconds(waitMs));
                continue;
            }

            return response;
        }

        return {};
    }

    std::vector<std::string> WikiClient::getLinks(const std::string &title) const
    {
        {
            std::lock_guard<std::mutex> lock(cacheMutex);
            auto cached = cache.find(title);
            if (cached != cache.end())
                return cached->second;
        }

        std::string url =
            "https://en.wikipedia.org/w/api.php"
            "?action=query"
            "&prop=links"
            "&titles=" +
            urlEncode(title) +
            "&pllimit=max"
            "&redirects=1"
            "&format=json";

        nlohmann::json data = nlohmann::json::parse(fetchJson(url));

        std::vector<std::string> links;

        for (auto it = data["query"]["pages"].begin(); it != data["query"]["pages"].end(); ++it)
        {
            const auto &page = it.value();
            if (!page.contains("links"))
                continue;

            for (const auto &link : page["links"])
            {
                if (link["ns"] != 0)
                    continue;

                links.push_back(link["title"]);
            }
        }

        std::lock_guard<std::mutex> lock(cacheMutex);
        cache[title] = links;
        return links;
    }

    std::vector<std::string> WikiClient::getBacklinks(const std::string &title) const
    {
        std::string resolved = resolveTitle(title);

        {
            std::lock_guard<std::mutex> lock(cacheMutex);
            auto cached = backlinkCache.find(resolved);
            if (cached != backlinkCache.end())
                return cached->second;
        }

        std::string url =
            "https://en.wikipedia.org/w/api.php"
            "?action=query"
            "&list=backlinks"
            "&bltitle=" +
            urlEncode(resolved) +
            "&blnamespace=0"
            "&bllimit=max"
            "&format=json";

        nlohmann::json data = nlohmann::json::parse(fetchJson(url));

        std::vector<std::string> backlinks;

        for (const auto &link : data["query"]["backlinks"])
        {
            if (link["ns"] != 0)
                continue;

            backlinks.push_back(link["title"]);
        }

        std::lock_guard<std::mutex> lock(cacheMutex);
        backlinkCache[resolved] = backlinks;
        return backlinks;
    }

    std::string WikiClient::resolveTitle(const std::string &title) const
    {
        {
            std::lock_guard<std::mutex> lock(cacheMutex);
            auto cached = redirectCache.find(title);
            if (cached != redirectCache.end())
                return cached->second;
        }

        std::string url =
            "https://en.wikipedia.org/w/api.php"
            "?action=query"
            "&titles=" +
            urlEncode(title) +
            "&redirects=1"
            "&format=json";

        nlohmann::json data = nlohmann::json::parse(fetchJson(url));

        std::string resolved = title;
        if (data["query"].contains("redirects") && !data["query"]["redirects"].empty())
        {
            resolved = data["query"]["redirects"][0]["to"];
        }

        std::lock_guard<std::mutex> lock(cacheMutex);
        redirectCache[title] = resolved;
        return resolved;
    }

}
