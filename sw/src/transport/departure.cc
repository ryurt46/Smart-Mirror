#include "transport/departure.h"
#include <sstream>
#include <string>

Departure::Departure(const std::string& dest, int mins, const std::string& info, bool del)
    : destination(dest), minutes_until(mins), transfer_info(info), delayed(del) {
}

void Departure::set_destination(const std::string& dest) {
    destination = dest;
}

void Departure::set_minutes_until(int mins) {
    minutes_until = mins;
}

void Departure::set_delayed(bool del) {
    delayed = del;
}

void Departure::set_arrival_minutes(int total_time) {
    arrival_minutes = total_time;
}

std::string Departure::display() const {
    std::ostringstream oss;
    oss << destination;

    if (minutes_until >= 0) {
        oss << " | ";
        if (minutes_until == 0)
            oss << "Avgår nu";
        else
            oss << "Avgår om " + std::to_string(minutes_until) + " min";
    }

    if (arrival_minutes >= 0)
        oss << " | Ankomsttid " + std::to_string(arrival_minutes) + " min";

    if (!transfer_info.empty())
        oss << " | " + transfer_info;

    if (delayed)
        oss << " | Delayed";

    return oss.str();
}

int Departure::get_minutes_until() const {
    return minutes_until;
}

void Departure::set_transfer_info(const std::string& info) {
    transfer_info = info;
}

std::string Departure::get_route_summary() const {
    return route_summary;
}

std::string Departure::get_destination_station() const {
    return destination_station;
}
