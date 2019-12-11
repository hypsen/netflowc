#pragma once
#include <cstdint>

struct LogDateTime {
    uint16_t year;
    uint8_t month;
    uint16_t day;
    uint8_t hour;
    uint8_t minute;
    uint8_t period;
    bool operator== (const LogDateTime& other) const {
        return (year == other.year) && (month == other.month) && (day == other.day) && (hour == other.hour) && (minute/period == other.minute/period);
    }
    bool operator!= (const LogDateTime& other) const {
        return (year != other.year) || (month != other.month) || (day != other.day) || (hour != other.hour) || (minute/period != other.minute/period);
    }
    void operator= (const LogDateTime& other) {
        year = other.year;
        month = other.month;
        day = other.day;
        hour = other.hour;
        minute = other.minute;
        period = other.period;
    }
};
