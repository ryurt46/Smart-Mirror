#pragma once
#include <chrono>
#include <cstdint>
#include <map>
#include <string>

class ClockState {
public:
    void update();
    std::string formatter();
    int get_week_number(const std::tm& tm);
    std::string get_current_date() const;
    std::string get_current_day() const;
    std::string get_weekday_from_date(const std::string& date_str);

private:
    static const std::map<int, std::string> week_days;
    uint16_t week_number;
    std::string current_day;
    std::string current_time;
    std::string current_date;
};
