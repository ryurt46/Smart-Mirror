#include "weather.h"
#include "../clock/clock.h"
#include <ctime>
#include <curl/curl.h>
#include <curl/easy.h>
#include <iomanip>
#include <iostream>
#include <nlohmann/detail/value_t.hpp>
#include <nlohmann/json.hpp>
#include <string>

using json = nlohmann::json;

#include "weather.h"

const std::vector<HourlyForecast>& Weather::get_hourly_forecast() const {
    return hourly_forecast;
}

const std::vector<ForecastDay>& Weather::get_daily_forecast() const {
    return daily_forecast;
}

std::string Weather::today_summary() const {
    if (hourly_forecast.empty()) {
        return "No forecast available.";
    }

    const auto& first = hourly_forecast.front();
    std::string summary = "Temp: " + std::to_string(first.temperature) + "°C, ";
    summary += "Wind: " + std::to_string(first.wind_speed) + " m/s, ";
    summary += "Weather code: " + std::to_string(first.weather_code);
    return summary;
}

void Weather::update_from_json(const std::string& json_data) {
    parse_hourly_json(json_data);
    parse_daily_json(json_data);
}

void Weather::parse_hourly_json(const std::string& json_data) {
    auto j = json::parse(json_data);
    hourly_forecast.clear();
    for (auto& it : j["timeSeries"]) {
        HourlyForecast hf;
        hf.valid_time = it["validTime"];
        for (auto& ts : it["parameters"]) {
            std::string name = ts["name"];
            if (name == "t") {
                hf.temperature = ts["values"][0].get<float>();
            } else if (name == "ws") {
                hf.wind_speed = ts["values"][0].get<float>();
            } else if (name == "Wsymb2") {
                hf.weather_code = ts["values"][0].get<int>();
            }
        }
        hourly_forecast.push_back(hf);
    }
}

size_t Weather::write_callback(char* ptr, size_t size, size_t nmemb, void* userdata) {
    size_t total_size = size * nmemb;
    std::string* str = static_cast<std::string*>(userdata);
    str->append(ptr, total_size);
    return total_size;
}

std::string Weather::perform_curl_request(const std::string& url) {
    std::string curl_data = "";
    CURL* curl = curl_easy_init();

    if (curl) {
        // "https://opendata-download-metfcst.smhi.se/api/category/pmp3g/version/2/geotype/point/lon/17.98466/lat/59.34297/data.json"
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, Weather::write_callback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &curl_data);
        curl_easy_setopt(curl, CURLOPT_HTTPGET, 1L);
        auto CURL_code = curl_easy_perform(curl);
        curl_easy_cleanup(curl);
        if (CURL_code == CURLE_OK) {
            return curl_data;
        } else {
            std::cerr << "Failed to curl data\n";
        }
    }
    return "\n";
}

void Weather::parse_daily_json(const std::string& json_data) {
    auto j = json::parse(json_data);
    daily_forecast.clear();

    std::map<std::string, std::vector<HourlyForecast>> daily_map;

    // Samla timforecast per dag
    for (auto& it : j["timeSeries"]) {
        HourlyForecast hf;
        hf.valid_time = it["validTime"];
        for (auto& ts : it["parameters"]) {
            std::string name = ts["name"];
            if (name == "t")
                hf.temperature = ts["values"][0].get<float>();
            else if (name == "ws")
                hf.wind_speed = ts["values"][0].get<float>();
            else if (name == "Wsymb2")
                hf.weather_code = ts["values"][0].get<int>();
        }

        std::string date = hf.valid_time.substr(0, 10);
        daily_map[date].push_back(hf);
    }

    // Skapa daily_forecast
    for (auto& [date, hours] : daily_map) {
        ForecastDay fd;
        fd.date = date;

        float sum_wind = 0.0f;
        float min_temp = hours.front().temperature;
        float max_temp = hours.front().temperature;
        std::map<int, int> weather_count;

        for (auto& h : hours) {
            if (h.temperature < min_temp)
                min_temp = h.temperature;
            if (h.temperature > max_temp)
                max_temp = h.temperature;
            sum_wind += h.wind_speed;
            weather_count[h.weather_code]++;
        }

        // Hitta mest förekommande weather code
        int common_code = std::max_element(
                              weather_count.begin(), weather_count.end(),
                              [](const auto& a, const auto& b) { return a.second < b.second; })
                              ->first;

        fd.min_temperature = min_temp;
        fd.max_temperature = max_temp;
        fd.avg_wind_speed = sum_wind / hours.size();
        fd.most_common_weather_code = common_code;

        daily_forecast.push_back(fd);
    }
}

const HourlyForecast* Weather::get_current_hour(time_t now_utc) const {
    if (hourly_forecast.empty())
        return nullptr;

    for (auto& fore_cast : hourly_forecast) {
        time_t fore_cast_time = str_to_time_t(fore_cast.valid_time); // <-- str_to_time_t tar valid_time
        if (fore_cast_time >= now_utc) {
            return &fore_cast;
        }
    }
    return nullptr;
}

const ForecastDay* Weather::get_today(const std::string& today_date) const {
    for (const auto& fd : daily_forecast) {
        if (fd.date == today_date) {
            return &fd;
        }
    }
    return nullptr;
}

time_t Weather::str_to_time_t(const std::string& str) const {
    std::tm tm{};
    std::istringstream ss(str);
    ss >> std::get_time(&tm, "%Y-%m-%dT%H:%M:%S");
    if (ss.fail()) {
        return 0;
    }
    return timegm(&tm);
}
