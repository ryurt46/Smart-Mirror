#include "transport/departure_group.h"
#include "helpers/helper.h"
#include <cctype>
#include <format>
#include <iostream>
#include <string>

using json = nlohmann::json;

const std::map<std::string, std::string> DepartureGroup::jp_site_ids{
    {"Huvudsta", "9091001000009327"},
    {"Kista", "9091001000009302"},
    {"T-Centralen", "9091001000009001"},
    {"Tekniska Högskolan", "9091001000009204"}};

DepartureGroup::DepartureGroup(const std::string& from_station, const std::string& to_station)
    : from(from_station), to(to_station) {
}

std::pair<std::string, std::string> DepartureGroup::get_station_ids() const {
    auto it_from = jp_site_ids.find(from);
    auto it_to = jp_site_ids.find(to);
    if (it_from == jp_site_ids.end() || it_to == jp_site_ids.end()) {
        throw std::runtime_error("Invalid station name: " + from + " -> " + to);
    }
    return {it_from->second, it_to->second};
}

std::string DepartureGroup::build_url(const std::string& from_id, const std::string& to_id) const {
    return std::format("https://journeyplanner.integration.sl.se/v2/trips?"
                       "type_origin=any&name_origin={}&type_destination=any&name_"
                       "destination={}&calc_number_of_trips=3",
                       from_id, to_id);
}

std::vector<std::string> DepartureGroup::display(size_t n) const {
    std::vector<std::string> result;
    for (size_t i = 0; i < departures.size() && i < n; ++i) {
        result.push_back(departures[i].display());
    }
    return result;
}

Departure DepartureGroup::parse_journey(const nlohmann::json& journey) const {
    Departure d;
    int start_minutes = -1;
    int arrival_minutes = -1;
    bool delayed = false;
    std::string route_summary = from;
    std::vector<std::string> transfers;

    const auto& legs = journey["legs"];
    auto clean_station_name = [](const std::string& name) {
        auto pos = name.find(',');
        return (pos != std::string::npos) ? name.substr(0, pos) : name;
    };
    auto clean_transport_name = [](const std::string& name) {
        const std::string prefix = "Tunnelbana ";
        if (name.rfind(prefix, 0) == 0)
            return name.substr(prefix.size());
        return name;
    };

    for (size_t i = 0; i < legs.size(); ++i) {
        const auto& leg = legs[i];
        std::string dest_name = clean_station_name(leg["destination"]["name"].get<std::string>());

        if (i == 0) {
            std::string dep_time_iso = leg["origin"]["departureTimePlanned"].get<std::string>();
            start_minutes = parse_minutes(dep_time_iso);
            route_summary += " - " + dest_name;
        }

        if (i + 1 < legs.size()) {
            const auto& next_leg = legs[i + 1];
            if (next_leg.contains("transportation") &&
                next_leg["transportation"].contains("name")) {
                std::string t_name =
                    clean_transport_name(next_leg["transportation"]["name"].get<std::string>());
                if (next_leg["transportation"].contains("destination") &&
                    next_leg["transportation"]["destination"].contains("name")) {
                    t_name += " mot " +
                              next_leg["transportation"]["destination"]["name"].get<std::string>();
                }
                if (!t_name.empty())
                    transfers.push_back("Byt till " + t_name);
            }
        }

        if (leg.contains("realtimeStatus") && !leg["realtimeStatus"].empty()) {
            std::string status = leg["realtimeStatus"][0].get<std::string>();
            if (status != "MONITORED")
                delayed = true;
        }

        if (i == legs.size() - 1) {
            if (leg["destination"].contains("arrivalTimeEstimated"))
                arrival_minutes =
                    parse_minutes(leg["destination"]["arrivalTimeEstimated"].get<std::string>());
            else if (leg["destination"].contains("arrivalTimePlanned"))
                arrival_minutes =
                    parse_minutes(leg["destination"]["arrivalTimePlanned"].get<std::string>());
        }
    }

    if (start_minutes < 0)
        throw std::runtime_error("Invalid start time");

    d.set_destination(route_summary);
    d.set_minutes_until(start_minutes);
    d.set_arrival_minutes(arrival_minutes);
    d.set_delayed(delayed);

    std::ostringstream transfer_text;
    for (size_t i = 0; i < transfers.size(); ++i) {
        if (i > 0)
            transfer_text << " | ";
        transfer_text << transfers[i];
    }
    d.set_transfer_info(transfer_text.str());

    return d;
}

void DepartureGroup::update() {
    departures.clear();

    try {
        auto [from_id, to_id] = get_station_ids();
        std::string url = build_url(from_id, to_id);
        std::string response = http_get(url);
        json j = json::parse(response);

        if (!j.contains("journeys"))
            return;

        for (const auto& journey : j["journeys"]) {
            if (!journey.contains("legs") || journey["legs"].empty())
                continue;
            try {
                departures.push_back(parse_journey(journey));
            } catch (...) {
                continue;
            }
        }

    } catch (const std::exception& e) {
        std::cerr << "DepartureGroup update failed: " << e.what() << "\n";
    }
}

std::string DepartureGroup::get_name() const {
    return from + " - " + to;
}
