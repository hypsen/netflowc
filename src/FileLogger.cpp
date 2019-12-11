#include "FileLogger.hpp"

void FileLogger::writeToFile_(tm datetime, std::string msg)
{
    try {
        LogDateTime logdatetime{
            .year = static_cast<uint16_t>(datetime.tm_year + 1900),
            .month = static_cast<uint8_t>(datetime.tm_mon + 1),
            .day = static_cast<uint8_t>(datetime.tm_mday),
            .hour = static_cast<uint8_t>(datetime.tm_hour),
            .minute = static_cast<uint8_t>(datetime.tm_min),
            .period = period_
        };

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

        auto getDirPath = [&](LogDateTime& datetime) -> std::filesystem::path {
            std::filesystem::path dir = dir_;
            dir /= std::to_string(datetime.year);
            dir /= datetime.month < 10 ? "0" : "" + std::to_string(datetime.month);   
            dir /= datetime.day < 10 ? "0" : "" + std::to_string(datetime.day);   
            return dir;
        };

        auto getFilePath = [&](LogDateTime& datetime) -> std::filesystem::path {
            std::filesystem::path file = getDirPath(datetime);
            file /= datetime.hour < 10 ? "0" : "" + std::to_string(datetime.hour) + "_";

            file += (datetime.minute / datetime.period * datetime.period < 10) ? "0" : "";
            file += std::to_string(datetime.minute / datetime.period * datetime.period);

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