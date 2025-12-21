#include "clock/clock.h"
#include "transport/transport.h"
#include "weather/weather.h"
#include <iostream>

int main() {
    Weather weather;
    std::string url = "https://opendata-download-metfcst.smhi.se/api/category/pmp3g/version/2/geotype/point/lon/17.98466/lat/59.34297/data.json";

    std::string json_data = weather.perform_curl_request(url);
    if (json_data.empty()) {
        std::cerr << "Failed to fetch weather data.\n";
        return 1;
    }

    weather.update_from_json(json_data);
    ClockState clock;
    clock.update();

    const ForecastDay* today = weather.get_today(clock.get_current_date());
    if (today) {
        std::cout << clock.get_current_day() << "'s forecast: Min: " << today->min_temperature
                  << "°C, Max: " << today->max_temperature
                  << "°C, Avg wind: " << today->avg_wind_speed
                  << " m/s, Weather code: " << today->most_common_weather_code
                  << "\n\n";
    }

    // Kommande 7 dagars forecast
    const auto& daily = weather.get_daily_forecast();
    std::string today_date = clock.get_current_date();
    int future_days_count = 0;

    for (const auto& df : daily) {
        if (df.date == today_date)
            continue;

        std::string weekday = clock.get_weekday_from_date(df.date);

        std::cout << df.date << " (" << weekday << ")"
                  << " | Min: " << df.min_temperature
                  << "°C | Max: " << df.max_temperature
                  << "°C | Avg wind: " << df.avg_wind_speed
                  << " m/s | Weather code: " << df.most_common_weather_code
                  << "\n";

        future_days_count++;
        if (future_days_count >= 7)
            break;
    }

    std::cout << "\n=== Departures ===\n";

    // --- Avgångar från Huvudsta till T-Centralen ---
    DepartureGroup huv_tc("Huvudsta", "T-Centralen");
    huv_tc.update();

    // --- Avgångar från Huvudsta till Kista ---
    DepartureGroup huv_kis("Huvudsta", "Kista");
    huv_kis.update();

    // --- Avgångar från Huvudsta till KTH ---
    DepartureGroup huv_kth("Huvudsta", "Tekniska Högskolan");
    huv_kth.update();

    // Skriv ut
    std::cout << "\n=== Huvudsta → T-Centralen ===\n";
    for (const auto& dep : huv_tc.display(5)) {
        std::cout << dep << "\n";
    }

    std::cout << "\n=== Huvudsta → Kista ===\n";
    for (const auto& dep : huv_kis.display(5)) {
        std::cout << dep << "\n";
    }

    std::cout << "\n=== Huvudsta → KTH ===\n";
    for (const auto& dep : huv_kth.display(5)) {
        std::cout << dep << "\n";
    }

    return 0;
}
