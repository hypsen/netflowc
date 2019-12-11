#pragma once
#include "Logger.hpp"
#include "LogDateTime.hpp"
#include <thread>
#include <chrono>
#include <ctime>
#include <iostream>
#include <boost/lockfree/spsc_queue.hpp>
#include "nfv9_types.hpp"
#include "../externals/clickhouse-cpp/clickhouse/client.h"

class ClickHouseLogger : public Logger {
private:
    std::string db_;
    std::string host_;
    uint16_t port_;
    std::string username_;
    std::string password_;
    boost::lockfree::spsc_queue<nfv9::DBRecord> msgQueue_;
    bool taskDone_;
    std::thread task_;
public:
    ClickHouseLogger(std::string host, uint16_t port, std::string db, std::string username, std::string password, std::size_t queueSize) : 
        Logger(LoggerType::ClickHouse),
	    db_(db),
	    host_(host),
        port_(port),
	    username_(username),
	    password_(password),
        msgQueue_(queueSize),
        taskDone_(false),
        task_(&ClickHouseLogger::run, this)
    {}
    void run() override;
    bool push(const nfv9::DBRecord& msg);
    ~ClickHouseLogger() override
    {
        taskDone_ = true;
        wakeup();
        if (task_.joinable())
            task_.join();
    }
};
