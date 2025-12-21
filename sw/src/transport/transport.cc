#include "transport/transport.h"
#include "helpers/helper.h"
#include <format>
#include <nlohmann/json.hpp>
#include <string>
using json = nlohmann::json;

const std::map<std::string, std::string> jp_site_ids{
    {"Huvudsta", "9091001000009327"},
    {"Kista", "9091001000009302"},
    {"T-Centralen", "9091001000009001"},
    {"Tekniska Högskolan", "9091001000009204"}};

const std::map<std::string, std::vector<std::string>> DepartureGroup::destination_aliases{
    {"Kista", {"Hjulsta", "Kungsträdgården", "Kista"}},
    {"T-Centralen", {"Kungsträdgården", "T-Centralen"}},
    {"Tekniska Högskolan", {"Hjulsta", "Tekniska Högskolan"}}};

void Departure::setDestination(const std::string& dest) {
    destination = dest;
}

void Departure::setMinutesUntil(int mins) {
    minutes_until = mins;
}

void Departure::setDelayed(bool del) {
    delayed = del;
}

std::string Departure::display() const {
    if (delayed)
        return destination + " | " + std::to_string(minutes_until) + " min" + " | Delayed";
    else
        return destination + " | " + std::to_string(minutes_until) + "min";
}

std::vector<std::string> DepartureGroup::display(size_t n) const {
    std::vector<std::string> result;
    for (size_t i = 0; i < departures.size() && i < n; ++i) {
        result.push_back(departures[i].display());
    }
    return result;
}

void DepartureGroup::update() {
    departures.clear();

    std::string from_id = jp_site_ids.at(this->from);
    std::string to_id = jp_site_ids.at(this->to);

    std::string url = std::format(
        "https://journeyplanner.integration.sl.se/v2/trips?type_origin=any&name_origin={}&type_destination=any&name_destination={}&calc_number_of_trips=3",
        from_id, to_id);

    std::cout << "Fetching URL: " << url << "\n";

    std::string response = http_get(url);
    std::cout << "Response length: " << response.size() << "\n";

    try {
        json j = json::parse(response);

        if (!j.contains("journeys")) {
            std::cerr << "No journeys found\n";
            return;
        }

        for (const auto& journey : j["journeys"]) {
            if (!journey.contains("legs") || journey["legs"].empty())
                continue;

            const auto& first_leg = journey["legs"][0];

            if (!first_leg.contains("transportation") || !first_leg["transportation"].contains("product"))
                continue;

            std::string product = first_leg["transportation"]["product"]["name"].get<std::string>();
            std::transform(product.begin(), product.end(), product.begin(), ::tolower);
            if (product != "tunnelbana")
                continue;

            Departure d;
            d.setDestination(first_leg["destination"]["name"].get<std::string>());

            std::string dep_time_iso = first_leg["origin"]["departureTimePlanned"].get<std::string>();
            int minutes = parse_minutes(dep_time_iso);

            if (minutes < 0) {
                continue; // hoppa över avgångar som redan passerat
            }

            d.setMinutesUntil(minutes);

            // Realtidsstatus
            if (first_leg.contains("realtimeStatus") && first_leg["realtimeStatus"].is_array() && !first_leg["realtimeStatus"].empty()) {
                std::string status = first_leg["realtimeStatus"][0].get<std::string>();
                d.setDelayed(status != "MONITORED");
            } else {
                d.setDelayed(false);
            }

            departures.push_back(d);
        }
    } catch (const std::exception& e) {
        std::cerr << "Failed to parse journey JSON: " << e.what() << "\n";
    }

    std::cout << "Total departures: " << departures.size() << "\n";
}

int Departure::get_minutes_until() const {
    return minutes_until;
}
