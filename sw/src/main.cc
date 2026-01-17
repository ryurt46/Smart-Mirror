#include "clock/clock.h"
#include "http/http_server.h"
#include "transport/departure_group.h"
#include "weather/weather.h"
#include <filesystem>
#include <fstream>
#include <nlohmann/json.hpp>
#include <sstream>

using json = nlohmann::json;
namespace fs = std::filesystem;

int main() {
    Weather weather;
    ClockState clock;

    weather.update_from_json(weather.fetch_weather_json(59.34297, 17.98466));
    clock.update();

    DepartureGroup huv_tc("Huvudsta", "T-Centralen");
    DepartureGroup huv_kis("Huvudsta", "Kista");
    DepartureGroup huv_kth("Huvudsta", "Tekniska Högskolan");

    huv_tc.update();
    huv_kis.update();
    huv_kth.update();

    HttpServer server(8080);

    server.add_route("/clock", [&]() {
        json j{{"current_date", clock.get_current_date()},
               {"current_day", clock.get_current_day()},
               {"current_time", clock.get_current_time()},
               {"week_number", clock.get_week_number()}};
        return std::make_pair(j.dump(), "application/json");
    });

    server.add_route("/weather", [&]() {
        json j;
        if (auto t = weather.get_today(clock.get_current_date())) {
            j["today"] = {{"date", t->date},
                          {"min_temp", t->min_temperature},
                          {"max_temp", t->max_temperature},
                          {"avg_wind", t->avg_wind_speed},
                          {"weather_code", t->most_common_weather_code}};
        }
        json arr = json::array();
        for (auto& d : weather.get_daily_forecast())
            if (d.date != clock.get_current_date())
                arr.push_back({{"date", d.date},
                               {"min_temp", d.min_temperature},
                               {"max_temp", d.max_temperature},
                               {"avg_wind", d.avg_wind_speed},
                               {"weather_code", d.most_common_weather_code}});
        j["forecast"] = arr;
        return std::make_pair(j.dump(), "application/json");
    });

    server.add_route("/departures", [&]() {
        huv_tc.update();
        huv_kis.update();
        huv_kth.update();

        auto pack = [](const DepartureGroup& g) {
            json a = json::array();
            for (auto& s : g.display(5))
                a.push_back(s);
            return a;
        };

        json j = json::array();
        j.push_back({{"name", "Huvudsta - Kista"}, {"departures", pack(huv_kis)}});
        j.push_back({{"name", "Huvudsta - T-Centralen"}, {"departures", pack(huv_tc)}});
        j.push_back({{"name", "Huvudsta - KTH"}, {"departures", pack(huv_kth)}});

        return std::make_pair(j.dump(), "application/json");
    });

    server.add_route("/", [&]() {
        namespace fs = std::filesystem;

        fs::path root = fs::current_path();

        if (root.filename() == "build") {
            root = root.parent_path();
        }

        fs::path index = root / "frontend" / "index.html";

        std::ifstream file(index);
        if (!file) {
            return std::make_pair("<h1>index.html not found</h1><pre>" + index.string() + "</pre>",
                                  "text/html");
        }

        std::stringstream buffer;
        buffer << file.rdbuf();
        return std::make_pair(buffer.str(), "text/html");
    });

    server.add_route("/style.css", []() -> std::pair<std::string, std::string> {
        std::ifstream file("frontend/style.css");
        if (!file)
            return {"File not found", "text/css"};
        std::stringstream buffer;
        buffer << file.rdbuf();
        return {buffer.str(), "text/css"};
    });

    server.add_route("/app.js", []() -> std::pair<std::string, std::string> {
        std::ifstream file("frontend/app.js");
        if (!file)
            return {"File not found", "application/javascript"};
        std::stringstream buffer;
        buffer << file.rdbuf();
        return {buffer.str(), "application/javascript"};
    });

    auto get_content_type = [](const fs::path& file_path) {
        std::string ext = file_path.extension().string();
        if (ext == ".svg")
            return "image/svg+xml";
        if (ext == ".ico")
            return "image/x-icon";
        return "application/octet-stream";
    };

    fs::path icons_dir = "frontend/icons";
    for (auto& entry : fs::directory_iterator(icons_dir)) {
        if (entry.is_regular_file()) {
            fs::path file_path = entry.path();
            std::string route = "/icons/" + file_path.filename().string();
            std::string type = get_content_type(file_path);

            server.add_route(route, [file_path, type]() -> std::pair<std::string, std::string> {
                std::ifstream file(file_path, std::ios::binary);
                if (!file)
                    return {"File not found: " + file_path.string(), "text/plain"};
                std::stringstream buffer;
                buffer << file.rdbuf();
                return {buffer.str(), type};
            });
        }
    }
    server.start();
}
