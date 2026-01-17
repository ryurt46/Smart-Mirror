#pragma once
#include <cstdint>
#include <ctime>
#include <map>
#include <string>

class ClockState {
public:
    void update();
    uint8_t calculate_week_number(const std::tm& tm);
    std::string get_current_date() const;
    std::string get_current_day() const;
    std::string get_weekday_from_date(const std::string& date_str);
    std::string get_current_time() const;
    uint8_t get_week_number() const;

private:
    static const std::map<int, std::string> week_days;
    uint8_t week_number;
    std::string current_day;
    std::string current_time;
    std::string current_date;
};
