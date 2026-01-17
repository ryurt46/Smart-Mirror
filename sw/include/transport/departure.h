#pragma once
#include <string>

class Departure {
public:
    Departure() = default;
    Departure(const std::string& dest, int mins, const std::string& info, bool del = false);

    std::string display() const;

    void set_destination(const std::string& dest);
    void set_minutes_until(int mins);
    void set_delayed(bool del);
    void set_arrival_minutes(int total_time);
    void set_transfer_info(const std::string& info);

    int get_minutes_until() const;
    std::string get_route_summary() const;
    std::string get_destination_station() const;

private:
    std::string route_summary;
    std::string destination_station;
    std::string destination;
    int minutes_until;
    int arrival_minutes;
    bool delayed;
    std::string transfer_info;
};
