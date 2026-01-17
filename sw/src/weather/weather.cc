#include "helpers/helper.h"
#include <ctime>
#include <format>
#include <iomanip>
#include <nlohmann/json.hpp>
#include <sstream>
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
    std::ostringstream oss;
    oss << "Temp: " << first.temperature << "°C, "
        << "Wind: " << first.wind_speed << " m/s, "
        << "Weather code: " << first.weather_code;
    return oss.str();
}

void Weather::update_from_json(const std::string& json_data) {
    try {
        json j = json::parse(json_data);
        parse_hourly_json(j);
        parse_daily_json(j);
    } catch (const json::parse_error& e) {
        std::cerr << "Failed to parse weather JSON: " << e.what() << "\n";
    }
}

// refactor later

void Weather::parse_hourly_json(const json& j) {
    hourly_forecast.clear();
    for (auto& it : j["timeSeries"]) {
        HourlyForecast hf;
        hf.valid_time = it.value("validTime", "");
        for (auto& ts : it["parameters"]) {
            std::string name = ts.value("name", "");
            if (!ts.contains("values") || !ts["values"].is_array() || ts["values"].empty())
                continue;

            auto& v = ts["values"][0];

            if (name == "t" && v.is_number()) {
                hf.temperature = v.get<float>();
            } else if (name == "ws" && v.is_number()) {
                hf.wind_speed = v.get<float>();
            } else if (name == "Wsymb2" && v.is_number()) {
                hf.weather_code = v.get<int>();
            }
        }
        hourly_forecast.push_back(hf);
    }
}

// refactor this later
void Weather::parse_daily_json(const json& j) {
    daily_forecast.clear();
    std::map<std::string, std::vector<HourlyForecast>> daily_map;

    for (auto& it : j["timeSeries"]) {
        HourlyForecast hf;
        hf.valid_time = it.value("validTime", "");

        for (auto& ts : it["parameters"]) {
            std::string name = ts.value("name", "");
            if (!ts.contains("values") || !ts["values"].is_array() || ts["values"].empty())
                continue;

            auto& v = ts["values"][0];

            if (name == "t" && v.is_number())
                hf.temperature = v.get<float>();
            else if (name == "ws" && v.is_number())
                hf.wind_speed = v.get<float>();
            else if (name == "Wsymb2" && v.is_number())
                hf.weather_code = v.get<int>();
        }

        std::string date = hf.valid_time.substr(0, 10);
        daily_map[date].push_back(hf);
    }

    for (auto& [date, hours] : daily_map) {
        if (hours.empty())
            continue;

        ForecastDay fd;
        fd.date = date;
        float sum_wind = 0.0f;
        float min_temp = hours.front().temperature;
        float max_temp = hours.front().temperature;
        std::map<int, int> weather_count;

        for (auto& h : hours) {
            min_temp = std::min(min_temp, h.temperature);
            max_temp = std::max(max_temp, h.temperature);
            sum_wind += h.wind_speed;
            weather_count[h.weather_code]++;
        }

        int common_code = 0;
        if (!weather_count.empty())
            common_code =
                std::max_element(weather_count.begin(), weather_count.end(),
                                 [](const auto& a, const auto& b) { return a.second < b.second; })
                    ->first;

        fd.min_temperature = min_temp;
        fd.max_temperature = max_temp;
        fd.avg_wind_speed = sum_wind / hours.size();
        fd.most_common_weather_code = common_code;

        daily_forecast.push_back(fd);
    }
}

std::string Weather::perform_curl_request(const std::string& url) {
    return http_get(url);
}

const HourlyForecast* Weather::get_current_hour(time_t now_utc) const {
    if (hourly_forecast.empty())
        return nullptr;

    for (auto& fore_cast : hourly_forecast) {
        time_t fore_cast_time =
            str_to_time_t(fore_cast.valid_time); // <-- str_to_time_t tar valid_time
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

std::string Weather::fetch_weather_json(double lat, double lon) {
    std::string url =
        std::format("https://opendata-download-metfcst.smhi.se/api/category/pmp3g/version/2/"
                    "geotype/point/lon/{}/lat/{}/data.json",
                    lon, lat);
    return perform_curl_request(url);
}
