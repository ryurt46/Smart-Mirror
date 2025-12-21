#include "helper.h"
#include <chrono>
#include <iomanip>
#include <sstream>
#include <string>

size_t write_callback(char* ptr, size_t size, size_t nmemb, void* userdata) {
    size_t total_size = size * nmemb;
    std::string* str = static_cast<std::string*>(userdata);
    str->append(ptr, total_size);
    return total_size;
}

std::string http_get(const std::string& url) {
    CURL* curl = curl_easy_init();
    std::string response;

    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

        CURLcode res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            std::cerr << "curl_easy_perform() failed: "
                      << curl_easy_strerror(res) << "\n";
        }

        curl_easy_cleanup(curl);
    }

    return response;
}

int parse_minutes(const std::string& iso_time) {
    std::tm tm = {};
    std::istringstream ss(iso_time);
    ss >> std::get_time(&tm, "%Y-%m-%dT%H:%M:%S");
    if (ss.fail())
        return -1;

    std::time_t tt = timegm(&tm); // UTC

    auto tp = std::chrono::system_clock::from_time_t(tt);
    auto now = std::chrono::system_clock::now();

    auto diff_min = std::chrono::duration_cast<std::chrono::minutes>(tp - now).count();
    return static_cast<int>(diff_min);
}
