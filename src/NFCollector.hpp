#pragma once
#include <iostream>
#include <unordered_map>
#include <vector>
#include <thread>
#include <filesystem>
#include <chrono>
#include <boost/asio.hpp>
#include <boost/functional/hash.hpp>
#include <boost/lockfree/spsc_queue.hpp>
#include <boost/lockfree/spsc_queue.hpp>
#include <boost/program_options.hpp>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "Logger.hpp"
#include "ClickHouseLogger.hpp"
#include "ConsoleLogger.hpp"
#include "FileLogger.hpp"
#include "utils.hpp"
#include "nfv9_types.hpp"
#include "../externals/span/include/tcb/span.hpp"
#include "../externals/clickhouse-cpp/clickhouse/client.h"
#include "../externals/rapidjson/include/rapidjson/document.h"
#include "../externals/rapidjson/include/rapidjson/stringbuffer.h"
#include "../externals/rapidjson/include/rapidjson/writer.h"

#define RECV_BUFFER_SIZE 4096

using namespace nfv9;

class NFServer
{
private:
    using TemplateKey = std::tuple<bool, __uint128_t, decltype(NetflowHeader::sourceId), decltype(TemplateHeader::templateId)>;
    using TemplateValue = std::vector<TemplateRecord>;
    struct TemplateKeyHash {
        std::size_t operator()(const TemplateKey & key) const
        {
            return boost::hash_value(key);
        }
    };

    __uint128_t senderAddrToUInt128_();
    void checkPacketLimits_(void* p, uint16_t bytes);
    uint16_t processTemplateFlowSet_(NetflowHeader* netflowHeader, FlowSetHeader* flowSetHeader);
    uint16_t processOptionsTemplateFlowSet_(NetflowHeader* netflowHeader, FlowSetHeader* flowSetHeader);
    void netflowToJSON_(uint16_t count, void* dataCursor, const TemplateRecord& record);
    void netflowToDBStruct_(void* dataCursor, const TemplateRecord& record, nfv9::DBRecord& dbStruct);
    uint16_t processDataFlowSet_(NetflowHeader* netflowHeader, FlowSetHeader* flowSetHeader);
    void processPacket_();
    void doReceive_();

    boost::asio::ip::udp::socket                socket_;
    boost::asio::ip::udp::endpoint              senderEndpoint_;
    std::array<char, RECV_BUFFER_SIZE>          recvBuf_;
    uint64_t                                    packets_ = 0;
    std::size_t                                 bytesTransferred_;
    std::unordered_map<TemplateKey, TemplateValue, TemplateKeyHash> NFV9Templates;

    rapidjson::StringBuffer JSONout;
    rapidjson::Writer<rapidjson::StringBuffer>  JSONwriter;

    std::vector<std::unique_ptr<Logger>>        loggers_;
    bool needJSONOutput_;
    bool needDBOutput_;
public:
    NFServer(boost::asio::io_context& io_context, const boost::asio::ip::udp::endpoint& endpoint);
    void run();
    void addConsoleLogger(std::size_t queueSize);
    void addFileLogger(fs::path dir, uint8_t fileLoggerPeriod, std::size_t queueSize);
    void addClickHouseLogger(std::string host, uint16_t port, std::string db, std::string username, std::string password, std::size_t queueSize);
    std::size_t getLoggersCount();
};
