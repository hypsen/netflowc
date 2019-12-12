#pragma once
#include <cstdint>
#include <ctime>

struct LogDateTime {
    uint16_t year;
    uint8_t month;
    uint16_t day;
    uint8_t hour;
    uint8_t minute;
    uint8_t period;
    LogDateTime() : year(0), month(0), day(0), hour(0), minute(0), period(0) {}
    LogDateTime(uint8_t tperiod) : year(0), month(0), day(0), hour(0), minute(0), period(tperiod) {}
    LogDateTime(tm& t, uint8_t tperiod) : 
        year(static_cast<uint16_t>(t.tm_year + 1900)),
        month(static_cast<uint8_t>(t.tm_mon + 1)),
        day(static_cast<uint8_t>(t.tm_mday)),
        hour(static_cast<uint8_t>(t.tm_hour)),
        minute(static_cast<uint8_t>(t.tm_min)),
        period(tperiod)
    {}
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
