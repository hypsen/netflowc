#include "Logger.hpp"

LoggerType Logger::getLoggerType()
{
    return loggerType_;
}

void Logger::wakeup()
{
    queueCV_.notify_one();
}