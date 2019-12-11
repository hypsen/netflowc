#include "ConsoleLogger.hpp"

void ConsoleLogger::run()
{
    try {
        std::mutex wakeup_mutex;
        std::string record;

        while (!taskDone_) {
            try {
                std::unique_lock<std::mutex> ul(wakeup_mutex);
                queueCV_.wait(ul);

                while (msgQueue_.pop(record)) {
                    std::cout << record << std::endl;
                }
            }
            catch (...) {}
        }
    }
    catch (...) {}
}

bool ConsoleLogger::push(const std::string& msg)
{
    return msgQueue_.push(msg);
}
