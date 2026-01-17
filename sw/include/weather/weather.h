#pragma once
#include <ctime>
#include <nlohmann/json_fwd.hpp>
#include <string>
#include <vector>

struct HourlyForecast {
    std::string valid_time; // t.ex. "2025-12-20T15:00:00Z"
    float temperature;      // °C
    float wind_speed;       // m/s
    int weather_code;       // SMHI weather symbol code

    HourlyForecast() : temperature(0.0f), wind_speed(0.0f), weather_code(0) {
    }
};

struct ForecastDay {
    std::string date; // "YYYY-MM-DD"
    float min_temperature;
    float max_temperature;
    float avg_wind_speed;
    int most_common_weather_code;

    ForecastDay()
        : min_temperature(0.0f), max_temperature(0.0f), avg_wind_speed(0.0f),
          most_common_weather_code(0) {
    }
};

class Weather {
public:
    Weather() = default;

    void update_from_json(const std::string& json_data);

    std::string today_summary() const;

    const std::vector<HourlyForecast>& get_hourly_forecast() const;

    const std::vector<ForecastDay>& get_daily_forecast() const;

    std::string fetch_weather_json(double lat, double lon);
    static size_t write_callback(char* ptr, size_t size, size_t nmemb, void* userdata);
    std::string perform_curl_request(const std::string& url);

    const ForecastDay* get_today(const std::string& today_date) const;
    const HourlyForecast* get_current_hour(time_t now_utc) const;
    time_t str_to_time_t(const std::string& str) const;

private:
    std::vector<HourlyForecast> hourly_forecast;
    std::vector<ForecastDay> daily_forecast;

    void parse_hourly_json(const nlohmann::json& j);
    void parse_daily_json(const nlohmann::json& j);
    void aggregate_daily();
};
