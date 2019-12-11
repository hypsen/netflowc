#pragma once
#include "Logger.hpp"
#include <thread>
#include <mutex>
#include <iostream>
#include <boost/lockfree/spsc_queue.hpp>

class ConsoleLogger : public Logger {
private:
    boost::lockfree::spsc_queue<std::string> msgQueue_;
    bool taskDone_;
    std::thread task_;
public:
    ConsoleLogger(std::size_t queueSize) :
        Logger(LoggerType::Console),
        msgQueue_(queueSize),
        taskDone_(false),
        task_(&ConsoleLogger::run, this)
    {}
    void run() override;
    bool push(const std::string& msg);
    ~ConsoleLogger() override
    {
        taskDone_ = true;
        wakeup();
        if (task_.joinable())
            task_.join();
    }
};
