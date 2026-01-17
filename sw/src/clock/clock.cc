#include <clock/clock.h>
#include <ctime>
#include <iomanip>
#include <string>

const std::map<int, std::string> ClockState::week_days{{0, "Söndag"}, {1, "Måndag"},  {2, "Tisdag"},
                                                       {3, "Onsdag"}, {4, "Torsdag"}, {5, "Fredag"},
                                                       {6, "Lördag"}};

uint8_t ClockState::calculate_week_number(const std::tm& tm) {
    char buf[4];
    std::strftime(buf, sizeof(buf), "%V", &tm);
    return static_cast<uint8_t>(std::stoi(buf));
}

void ClockState::update() {
    std::time_t now = std::time(nullptr);
    std::tm* local_time = std::localtime(&now);

    int current_year = local_time->tm_year + 1900;
    int current_month = local_time->tm_mon + 1;
    int current_day_number = local_time->tm_mday;
    current_date = std::to_string(current_year) + "-" + std::to_string(current_month) + "-" +
                   std::to_string(current_day_number);

    int hour = local_time->tm_hour;
    int minute = local_time->tm_min;
    current_time = std::to_string(hour) + ":" + std::to_string(minute);

    current_day = week_days.at(local_time->tm_wday);

    week_number = calculate_week_number(*local_time);
}

std::string ClockState::get_current_day() const {
    return current_day;
}

std::string ClockState::get_current_date() const {
    return current_date;
}

std::string ClockState::get_weekday_from_date(const std::string& date_str) {
    std::tm tm{};
    std::istringstream ss(date_str);
    ss >> std::get_time(&tm, "%Y-%m-%d"); // parse YYYY-MM-DD
    if (ss.fail())
        return "Unknown";

    std::time_t time = std::mktime(&tm);
    tm = *std::localtime(&time);

    auto it = week_days.find(tm.tm_wday);
    if (it != week_days.end())
        return it->second;

    return "Unknown";
}

std::string ClockState::get_current_time() const {
    return current_time;
}

uint8_t ClockState::get_week_number() const {
    return week_number;
}
