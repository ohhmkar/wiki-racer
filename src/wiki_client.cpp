#include "wiki/wiki_client.hpp"

#include <curl/curl.h>
#include <nlohmann/json.hpp>

#include <stdexcept>
#include <string>

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

    std::vector<std::stirng> WikiClient::getLinks(const std::string &title) const
    {
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

        curl_easy_cleanup(curl);

        return {};
    }

}
