#pragma once
#include <condition_variable>

enum class LoggerType {
    Console,
    File,
    ClickHouse
};

class Logger {
protected:
    std::condition_variable queueCV_;
    LoggerType loggerType_;
public:
    Logger(LoggerType loggerType) : loggerType_(loggerType) {}
    virtual void run() = 0;
    LoggerType getLoggerType();
    void wakeup();
    virtual ~Logger() {};
};

