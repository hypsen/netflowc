#include "FileLogger.hpp"

void FileLogger::writeToFile_(tm datetime, std::string msg)
{
    try {
        LogDateTime logdatetime(datetime, period_);

        if (logdatetime == datetimePrevious_) {
            if (ofsPrevious_.is_open())
                ofsPrevious_ << msg << std::endl;
            return;
        }
        if (logdatetime == datetimeCurrent_) {
            if (ofsCurrent_.is_open())
                ofsCurrent_ << msg << std::endl;
            return;
        }

        if (ofsPrevious_.is_open())
            ofsPrevious_.close();
        if (ofsCurrent_.is_open())
            ofsCurrent_.close();

        datetimePrevious_ = datetimeCurrent_;
        datetimeCurrent_ = logdatetime;

        auto getDirPath = [&](LogDateTime& logdatetime) -> std::filesystem::path {
            std::filesystem::path dir = dir_;
            dir /= std::to_string(logdatetime.year);
            dir /= logdatetime.month < 10 ? "0" : "" + std::to_string(logdatetime.month);   
            dir /= logdatetime.day < 10 ? "0" : "" + std::to_string(logdatetime.day);   
            return dir;
        };

        auto getFilePath = [&](LogDateTime& logdatetime) -> std::filesystem::path {
            std::filesystem::path file = getDirPath(logdatetime);
            file /= logdatetime.hour < 10 ? "0" : "" + std::to_string(logdatetime.hour) + "_";

            file += (logdatetime.minute / logdatetime.period * logdatetime.period < 10) ? "0" : "";
            file += std::to_string(logdatetime.minute / logdatetime.period * logdatetime.period);

            return file;
        };

        if (datetimePrevious_.year != 0) {
            std::filesystem::create_directories(getDirPath(datetimePrevious_));
            ofsPrevious_.open(getFilePath(datetimePrevious_), std::ios_base::app);
        }
        if (datetimeCurrent_.year != 0) {
            std::filesystem::create_directories(getDirPath(datetimeCurrent_));
            ofsCurrent_.open(getFilePath(datetimeCurrent_), std::ios_base::app);
        }

        if (ofsCurrent_.is_open())
            ofsCurrent_ << msg << std::endl;
    }
    catch (...) {}
}

void FileLogger::run()
{
    try {
        std::mutex wakeup_mutex;
        std::pair<std::chrono::milliseconds, std::string> record;

        while (!taskDone_) {
            try {
                std::unique_lock<std::mutex> ul(wakeup_mutex);
                queueCV_.wait(ul);

                while (msgQueue_.pop(record)) {
                    auto seconds = static_cast<time_t>(std::chrono::duration_cast<std::chrono::seconds>(record.first).count());
                    auto tm = *localtime(&seconds);
                    writeToFile_(tm, record.second);
                }
            }
            catch (...) {}
        }
    }
    catch (...) {}
}

bool FileLogger::push(std::chrono::milliseconds received, const std::string& msg)
{
    return msgQueue_.push(std::make_pair(received, msg));
}