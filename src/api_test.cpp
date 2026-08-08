#include <curl/curl.h>

#include <curl/easy.h>
#include <iostream>
#include <string>

size_t writeCallback(char *contents, size_t size, size_t nmemb, void *userp) {

  size_t totalSize = size * nmemb;

  std::string *response = static_cast<std::string *>(userp);

  response->append(contents, totalSize);
  return totalSize;
}

int main() {

  CURL *curl = curl_easy_init();

  if (!curl) {
    cout << "curl init failed\n";
    return 1;
  }

  std::string response;

  curl_easy_setopt(
      curl, CURLOP_URL,
      "https://en.wikipedia.org/w/"
      "api.php?action=query&prop=links&titles=Cat&pllimit=max&format=json");

  curl_easy_setopt(cur, CURLOPT_WRITEFUNCTION, writeCallback);

  curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

  CURLcode result = curl_easy_perform(curl);

  if (result != CURLE_OK) {
    std::cerr << "req failed" << curl_easy_strerror(result) << "\n";

    curl_easy_cleanup(curl);
    return 1;
  }

  curl_easy_cleanup(curl);

  std::cout << response << "\n";

  return 0;
}
