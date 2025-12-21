#pragma once
#include <cstdint>
#include <map>
#include <string>
#include <vector>

class Departure {
public:
    Departure() = default;
    Departure(const std::string& dest, int mins, bool del = false)
        : destination(dest), minutes_until(mins), delayed(del) {}

    // Returnerar textsträng redo för UI
    std::string display() const;

    void setDestination(const std::string& dest);
    void setMinutesUntil(int mins);
    void setDelayed(bool del);

    int get_minutes_until() const;

private:
    std::string destination;
    int minutes_until;
    bool delayed;
};

class DepartureGroup {
public:
    DepartureGroup() = default;
    DepartureGroup(const std::string& from_station, const std::string& to_station)
        : from(from_station), to(to_station) {}

    // Uppdaterar listan med nya avgångar
    void update();

    // Returnerar de första n avgångarna som strängar redo för UI
    std::vector<std::string> display(size_t n = 2) const;

private:
    std::string from;
    std::string to;
    std::vector<Departure> departures;
    static const std::map<std::string, uint32_t> site_ids;
    static const std::map<std::string, std::vector<std::string>> destination_aliases;
};
