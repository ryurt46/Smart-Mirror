#pragma once
#include "departure.h"
#include <map>
#include <nlohmann/json.hpp>
#include <string>
#include <vector>

class DepartureGroup {
public:
    DepartureGroup() = default;
    DepartureGroup(const std::string& from_station, const std::string& to_station);

    void update();
    std::vector<std::string> display(size_t n = 2) const;
    std::pair<std::string, std::string> get_station_ids() const;
    std::string build_url(const std::string& from_id, const std::string& to_id) const;
    Departure parse_journey(const nlohmann::json& journey) const;
    std::string get_name() const;

private:
    std::string from;
    std::string to;
    std::vector<Departure> departures;
    static const std::map<std::string, std::string> jp_site_ids;
};
