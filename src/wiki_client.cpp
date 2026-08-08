#include "wiki/wiki_client.hpp"

#include <curl/curl.h>
#include <nlohmann/json.hpp>

#include <stdexcept>
#include <string>
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
    }

    std::vector<std::string> WikiClient::getLinks(const std::string &title) const
    {

        auto cached = cache.find(title);
        if (cached != cache.end())
        {
            reutn cached->second;
        }
        CURL *curl = curl_easy_init();

        if (!curl)
        {
            throw std::runtime_error("curl init failed");
        }

        char *encodedTitle = curl_easy_escape(curl, title.c_str(), title.size());

        if (!encodedTitle)
        {
            curl_easy_cleanup(curl);
            throw std::runtime_error("failed to encode title");
        }

        std::string url =
            "https://en.wikipedia.org/w/api.php"
            "?action=query"
            "&prop=links"
            "&titles=" +
            std::string(encodedTitle) +
            "&pllimit=max"
            "&format=json";

        curl_free(encodedTitle);

        std::string response;

        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

        curl_easy_setopt(curl, CURLOPT_USERAGENT, "wiki-racer/0.1 (https://github.com/ohhmkar/wiki-racer)");

        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);

        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

        CURLcode result = curl_easy_perform(curl);

        if (result != CURLE_OK)
        {
            curl_easy_cleanup(curl);

            throw std::runtime_error(
                curl_easy_strerror(result));
        }

        curl_easy_cleanup(curl);

        nlohmann::json data = nlohmann::json::parse(response);

        std::vector<std::string> links;

        for (auto it = data["query"]["pages"].begin(); it != data["query"]["pages"].end(); ++it)
        {
            const auto &page = it.value();
            if (!page.contains("links"))
            {
                continue;
            }

            for (const auto &link : page["links"])
            {
                if (link["ns"] != 0)
                {
                    continue;
                }

                links.push_back(link["title"]);
            }
        }

        cache[title] = links;
        return links;
    }

}
