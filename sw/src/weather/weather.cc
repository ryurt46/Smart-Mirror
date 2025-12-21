#include "helpers/helper.h"
#include <ctime>
#include <iomanip>
#include <nlohmann/detail/value_t.hpp>
#include <nlohmann/json.hpp>
#include <string>
#include <weather/weather.h>

using json = nlohmann::json;

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

std::string Weather::perform_curl_request(const std::string& url) {
    return http_get(url);
}

void Weather::parse_daily_json(const std::string& json_data) {
    auto j = json::parse(json_data);
    daily_forecast.clear();

    std::map<std::string, std::vector<HourlyForecast>> daily_map;

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
