#pragma once
#include "Logger.hpp"
#include "LogDateTime.hpp"
#include <thread>
#include <filesystem>
#include <chrono>
#include <ctime>
#include <fstream>
#include <boost/lockfree/spsc_queue.hpp>

namespace fs = std::filesystem;

class FileLogger : public Logger {
private:
    std::ofstream ofsPrevious_;
    std::ofstream ofsCurrent_;
    LogDateTime datetimePrevious_;
    LogDateTime datetimeCurrent_;
    fs::path dir_;
    uint8_t period_;
    boost::lockfree::spsc_queue<std::pair<std::chrono::milliseconds, std::string>> msgQueue_;
    bool taskDone_;
    std::thread task_;

    void writeToFile_(tm datetime, std::string);
public:
    FileLogger(fs::path dir, uint8_t period, std::size_t queueSize) :
	    Logger(LoggerType::File),
        datetimePrevious_{0, 0, 0, 0, 0, period},
        datetimeCurrent_{0, 0, 0, 0, 0, period},
	    dir_(dir),
	    period_(period),
        msgQueue_(queueSize),
        taskDone_(false),
        task_(&FileLogger::run, this)
    {}
    void run() override;
    bool push(std::chrono::milliseconds received, const std::string& msg);
    ~FileLogger() override
    {
        taskDone_ = true;
        wakeup();
        if (task_.joinable())
            task_.join();
    }
};


