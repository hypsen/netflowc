#include "NFCollector.hpp"

#define UNUSED(v) (void)v
#define NETFLOW_VERSION 9
#define DATA_FLOWSET_START_ID 256
#define UNKNOWN_FIELD_STRING_LEN 256

using namespace nfv9;

NFServer::NFServer(boost::asio::io_context& io_context, const boost::asio::ip::udp::endpoint& endpoint) :
    socket_(io_context, endpoint),
    JSONwriter(JSONout),
    needJSONOutput_(false),
    needDBOutput_(false)
{
    JSONout.Reserve(1024);
}

void NFServer::run()
{
    doReceive_();
}

void NFServer::addConsoleLogger(std::size_t queueSize)
{
    loggers_.emplace_back(std::make_unique<ConsoleLogger>(queueSize));
    needJSONOutput_ = true;
}

void NFServer::addFileLogger(fs::path dir, uint8_t fileLoggerPeriod, std::size_t queueSize)
{
    loggers_.emplace_back(std::make_unique<FileLogger>(dir, fileLoggerPeriod, queueSize));
    needJSONOutput_ = true;
}

void NFServer::addClickHouseLogger(std::string host, uint16_t port, std::string db, std::string username, std::string password, std::size_t queueSize)
{
    loggers_.emplace_back(std::make_unique<ClickHouseLogger>(host, port, db, username, password, queueSize));
    needDBOutput_ = true;
}

std::size_t NFServer::getLoggersCount() {
    return loggers_.size();
}

__uint128_t NFServer::senderAddrToUInt128_()
{
    if (senderEndpoint_.address().is_v4())
        return senderEndpoint_.address().to_v4().to_uint();

    auto ipv6AddrBytes = senderEndpoint_.address().to_v6().to_bytes();
    return *reinterpret_cast<__uint128_t*>(ipv6AddrBytes.data()); 
}

void NFServer::doReceive_()
{
    packets_++;

    socket_.async_receive_from(
        boost::asio::buffer(recvBuf_),
        senderEndpoint_,
        [this](const boost::system::error_code& error, std::size_t bytesTransferred) {
            if (!error) {
                bytesTransferred_ = bytesTransferred;
                processPacket_();
                doReceive_();
            }
        }
    );
}

void NFServer::checkPacketLimits_(void* p, uint16_t bytes)
{
    if ((static_cast<char*>(p) + bytes) > (recvBuf_.data() + bytesTransferred_))
        throw(Nf_Server_Exception::END_OF_PACKET);
}

uint16_t NFServer::processTemplateFlowSet_(NetflowHeader* netflowHeader, FlowSetHeader* flowSetHeader)
{
    uint16_t count = 0;
    uint8_t* startCursorPosition = reinterpret_cast<uint8_t*>(flowSetHeader + 1);
    uint8_t* templateCursor = startCursorPosition;

    while (static_cast<uint16_t>(templateCursor - startCursorPosition) < (ntohs(flowSetHeader->length) - sizeof(FlowSetHeader))) {
        TemplateHeader* templateHeader = reinterpret_cast<TemplateHeader*>(templateCursor);

        templateCursor += sizeof(TemplateHeader);
        tcb::span<TemplateRecord> templateRecords{reinterpret_cast<TemplateRecord*>(templateCursor),
                                                  ntohs(templateHeader->fieldCount)};

        TemplateValue nfTemplate(ntohs(templateHeader->fieldCount));
        checkPacketLimits_(templateCursor, sizeof(TemplateRecord) * ntohs(templateHeader->fieldCount));
        uint16_t templateSize = 0;
        for (const auto& templateRecord : templateRecords) {
            nfTemplate.emplace_back(TemplateRecord{ntohs(templateRecord.fieldType), ntohs(templateRecord.fieldLength)});
            templateSize += ntohs(templateRecord.fieldLength);
        }
        nfTemplate.push_back(TemplateRecord{0, templateSize});

        auto sourceId = ntohl(netflowHeader->sourceId);
        auto templateId = ntohs(templateHeader->templateId);
        NFV9Templates[TemplateKey(senderEndpoint_.address().is_v6(), senderAddrToUInt128_(), sourceId, templateId)] = nfTemplate;

        templateCursor += sizeof(TemplateRecord) * ntohs(templateHeader->fieldCount);
        ++count;
    }

    return count;
}

uint16_t NFServer::processOptionsTemplateFlowSet_(NetflowHeader* netflowHeader, FlowSetHeader* optionsTemplateFlowSetHeader)
{
    /* ToDo */
    UNUSED(netflowHeader);
    UNUSED(optionsTemplateFlowSetHeader);
    throw(Nf_Server_Exception::END_OF_PACKET);
    return 1;
}

void NFServer::netflowToJSON_(uint16_t count, void* dataCursor, const TemplateRecord& record)
{
    auto fieldInfoIt = NETFLOW_V9_TYPES.find(record.fieldType);

    auto writeUnknownField = [&]() {
            char macString[UNKNOWN_FIELD_STRING_LEN * 2];
            std::size_t recordLen = record.fieldLength < UNKNOWN_FIELD_STRING_LEN ? record.fieldLength : UNKNOWN_FIELD_STRING_LEN;
            byteArrayToCharString(reinterpret_cast<uint8_t*>(dataCursor), recordLen, macString, false);
            JSONwriter.String(macString);
    };

    if (fieldInfoIt != NETFLOW_V9_TYPES.end()) {
        JSONwriter.String(fieldInfoIt->second.first.c_str());
        switch (fieldInfoIt->second.second)
        {
        case Netflow_V9_Print_Function_Type::NUMBER : {
            switch (record.fieldLength)
            {
            case 1:
                JSONwriter.Uint(*reinterpret_cast<uint8_t*>(dataCursor));
                break;
            case 2:
                JSONwriter.Uint(ntohs(*reinterpret_cast<uint16_t*>(dataCursor)));
                break;
            case 4:
                JSONwriter.Uint(ntohl(*reinterpret_cast<uint32_t*>(dataCursor)));
                break;
            case 8:
                JSONwriter.Uint64(ntohll(*reinterpret_cast<uint64_t*>(dataCursor)));
                break;

            default:
                /* ToDo write number variadic length */
                JSONwriter.Uint(0);
            }
            break;
        }
        case Netflow_V9_Print_Function_Type::IPV4 : {
            char ipv4Str[INET_ADDRSTRLEN];
            if (inet_ntop(AF_INET, reinterpret_cast<in_addr*>(dataCursor), ipv4Str, sizeof(ipv4Str)) != NULL)
                JSONwriter.String(ipv4Str);
            else
                JSONwriter.String("Error");
            break;
        }
        case Netflow_V9_Print_Function_Type::IPV6 : {
            char ipv6Str[INET6_ADDRSTRLEN];
            if (inet_ntop(AF_INET6, reinterpret_cast<in_addr*>(dataCursor), ipv6Str, sizeof(ipv6Str)) != NULL)
                JSONwriter.String(ipv6Str);
            else
                JSONwriter.String("Error");
            break;
        }
        case Netflow_V9_Print_Function_Type::NATEVENT : {
            switch (*reinterpret_cast<uint8_t*>(dataCursor)) {
            case 1 :
                JSONwriter.String("ADD");
                break;
            case 2 :
                JSONwriter.String("DEL");
                break;
            default :
                JSONwriter.String("UNKNOWN");
            }
            break;
        }
        case Netflow_V9_Print_Function_Type::MAC : {
            char macString[18];
            byteArrayToCharString(reinterpret_cast<uint8_t*>(dataCursor), 6, macString, true, ':');
            JSONwriter.String(macString);
            break;               
        }
        case Netflow_V9_Print_Function_Type::DATETIME : {
            /* ToDo */
            JSONwriter.String("DATETIME");
            break;
        }
        default:
            writeUnknownField();
        }
    }
    else {
        JSONwriter.String(std::string("UNKNOWN_FIELD" + std::to_string(count)).c_str());
        writeUnknownField();
    }
}

void NFServer::netflowToDBStruct_(void* dataCursor, const TemplateRecord& record, nfv9::DBRecord& dbStruct)
{
    auto getUInt8 = [&]() -> uint8_t {
        return *static_cast<uint8_t*>(dataCursor);
    };

    auto getUInt16 = [&]() -> uint16_t {
        switch (record.fieldLength)
        {
        case 1:
            return getUInt8();
        case 2:
            return ntohs(*static_cast<uint16_t*>(dataCursor));
        }
        return 0;
    };

    auto getUInt32 = [&]() -> uint32_t {
        switch (record.fieldLength) {
            case 1: case 2:
                return getUInt16();
            case 4:
                return ntohl(*static_cast<uint32_t*>(dataCursor));
        }
        return 0;
    };

    auto getUInt64 = [&]() -> uint64_t {
        switch (record.fieldLength) {
            case 1: case 2: case 4:
                return getUInt32();
            case 8:
                return ntohll(*static_cast<uint64_t*>(dataCursor));
        }
        return 0;
    };

    dbStruct.SENDER = senderAddrToUInt128_();

    switch (record.fieldType) {
        // uint32_t IPV4_SRC_ADDR
        case 8:  dbStruct.IPV4_SRC_ADDR = getUInt32(); break;
        // uint32_t IPV4_DST_ADDR
        case 12:  dbStruct.IPV4_DST_ADDR = getUInt32(); break;
        // uint8_t  TOS
        case 5:  dbStruct.TOS = getUInt8(); break;
        // uint8_t  PROTOCOL
        case 4:  dbStruct.PROTOCOL = getUInt8(); break;
        // uint16_t L4_SRC_PORT
        case 7:  dbStruct.L4_SRC_PORT = getUInt16(); break;
        // uint16_t L4_DST_PORT
        case 11:  dbStruct.L4_DST_PORT = getUInt16(); break;
        // uint16_t ICMP_TYPE;
        case 32:  dbStruct.ICMP_TYPE = getUInt16(); break;
        // uint32_t INPUT_SNMP;
        case 10:  dbStruct.INPUT_SNMP = getUInt16(); break;
        // uint16_t SRC_VLAN;
        case 58:  dbStruct.SRC_VLAN = getUInt16(); break;
        // uint8_t  SRC_MASK;
        case 9: dbStruct.SRC_MASK = getUInt8(); break;
        // uint8_t  DST_MASK;
        case 13: dbStruct.DST_MASK = getUInt8(); break;
        // uint32_t SRC_AS;
        case 16: dbStruct.SRC_AS = getUInt32(); break;
        // uint32_t DST_AS;
        case 17: dbStruct.DST_AS = getUInt32(); break;
        // uint32_t IPV4_NEXT_HOP;
        case 15: dbStruct.IPV4_NEXT_HOP = getUInt32(); break;
        // uint8_t  TCP_FLAGS;
        case 6: dbStruct.TCP_FLAGS = getUInt8(); break;
        // uint32_t OUTPUT_SNMP;
        case 14: dbStruct.OUTPUT_SNMP = getUInt32(); break;
        // uint64_t IN_BYTES;
        case 1: dbStruct.IN_BYTES = getUInt64(); break;
        // uint64_t IN_PACKETS;
        case 2: dbStruct.IN_PACKETS = getUInt64(); break;
        // uint32_t FIRST_SWITCHED;
        case 22: dbStruct.FIRST_SWITCHED = getUInt32(); break;
        // uint32_t LAST_SWITCHED;
        case 21: dbStruct.LAST_SWITCHED = getUInt32(); break;
        // uint8_t  IP_PROTOCOL_VERSION;
        case 60: dbStruct.IP_PROTOCOL_VERSION = getUInt8(); break;
        // uint32_t NF9_BGP_IPV4_NEXT_HOP;
        case 18: dbStruct.NF9_BGP_IPV4_NEXT_HOP = getUInt32(); break;
        // uint8_t  DIRECTION;
        case 61: dbStruct.DIRECTION = getUInt8(); break;
        // uint64_t NF_F_EVENT_TIME_MSEC;
        case 323: dbStruct.NF_F_EVENT_TIME_MSEC = getUInt64(); break;
        // uint32_t NF_F_XLATE_SRC_ADDR_IPV4;
        case 225: dbStruct.NF_F_XLATE_SRC_ADDR_IPV4 = getUInt32(); break;
        // uint32_t NF_F_XLATE_DST_ADDR_IPV4;
        case 226: dbStruct.NF_F_XLATE_DST_ADDR_IPV4 = getUInt32(); break;
        // uint16_t NF_F_XLATE_SRC_PORT;
        case 227: dbStruct.NF_F_XLATE_SRC_PORT = getUInt16(); break;
        // uint16_t NF_F_XLATE_DST_PORT;
        case 228: dbStruct.NF_F_XLATE_DST_PORT = getUInt16(); break;
        // uint8_t  natEvent;
        case 230: dbStruct.natEvent = getUInt8(); break;
    }
}


uint16_t NFServer::processDataFlowSet_(NetflowHeader* netflowHeader, FlowSetHeader* flowSetHeader)
{
    auto sourceId = ntohl(netflowHeader->sourceId);
    auto templateId = ntohs(flowSetHeader->flowSetId);
    auto templateIt = NFV9Templates.find(TemplateKey(senderEndpoint_.address().is_v6(), senderAddrToUInt128_(), sourceId, templateId));

    if (templateIt == NFV9Templates.end()) {
        throw(Nf_Server_Exception::UNKNOWN_TEMPLATE);
    }

    uint16_t count = 0;
    uint8_t* startCursorPosition = reinterpret_cast<uint8_t*>(flowSetHeader + 1);
    uint8_t* dataCursor = startCursorPosition;

    nfv9::DBRecord dbStruct = nfv9::DBRecord();

    auto msgMilliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch());

    while (static_cast<uint16_t>(dataCursor - startCursorPosition) < (ntohs(flowSetHeader->length) - sizeof(FlowSetHeader))) {
        TemplateValue& nfTemplate = templateIt->second;
        checkPacketLimits_(dataCursor, nfTemplate.back().fieldLength);

        if (needJSONOutput_) {
            JSONout.Clear();
            JSONwriter.Reset(JSONout);
            JSONwriter.StartObject();
            JSONwriter.String("SENDER");
            JSONwriter.String(senderEndpoint_.address().to_string().c_str());
            JSONwriter.String("RECEIVED");
            JSONwriter.Uint64(msgMilliseconds.count());
        }

        uint16_t recordCount = 0;
        for (const auto& record : nfTemplate) {
            /* last record contain tempate size of all records and not need to convert*/
            if (record.fieldType != 0) {
                if (needJSONOutput_)
                    netflowToJSON_(recordCount, dataCursor, record);
                if (needDBOutput_)
                    netflowToDBStruct_(dataCursor, record, dbStruct);
                dataCursor += record.fieldLength;
                ++recordCount;
            }
        }

        if (needJSONOutput_) {
            JSONwriter.EndObject();
        }

        for (auto& logger : loggers_) {
            switch (logger->getLoggerType())
            {
                case LoggerType::Console : {
                    static_cast<ConsoleLogger*>(logger.get())->push(JSONout.GetString());
                    break;
                }
                case LoggerType::File : {
                    static_cast<FileLogger*>(logger.get())->push(msgMilliseconds, JSONout.GetString());
                    break;
                }
                case LoggerType::ClickHouse : {
                    dbStruct.RECEIVED_MSEC = msgMilliseconds.count();
                    static_cast<ClickHouseLogger*>(logger.get())->push(dbStruct);
                    break;
                }
            }
        }

        ++count;
    }

    for (auto& logger : loggers_) {
        logger->wakeup();
    }

    return count;
}

void NFServer::processPacket_()
{
    try {
        auto netflowHeader = reinterpret_cast<NetflowHeader*>(recvBuf_.data());

        if (ntohs(netflowHeader->versionNumber) != NETFLOW_VERSION)
            return;

        auto flowSetCursor = reinterpret_cast<uint8_t*>(netflowHeader) + sizeof(NetflowHeader);
        for (uint16_t flowSetN = 0; (flowSetN < ntohs(netflowHeader->count)) && 
                                    (static_cast<std::size_t>(flowSetCursor - reinterpret_cast<uint8_t*>(netflowHeader)) < bytesTransferred_);) {
            FlowSetHeader* flowSetHeader = reinterpret_cast<FlowSetHeader*>(flowSetCursor);
            switch (flowSetHeader->flowSetId)
            {
            /* Template FlowSet */
            case 0: {
                flowSetN += processTemplateFlowSet_(netflowHeader, flowSetHeader);
                break;
            }
            /* Options Template FLowSet */
            case 1: {
                flowSetN += processOptionsTemplateFlowSet_(netflowHeader, flowSetHeader);
                break;
            }
            /* Data FlowSet */
            default:
                if (ntohs(flowSetHeader->flowSetId) >= DATA_FLOWSET_START_ID)
                    flowSetN += processDataFlowSet_(netflowHeader, flowSetHeader);
                break;
            }
            flowSetCursor = flowSetCursor + ntohs(flowSetHeader->length);
        }
    }
    catch (Nf_Server_Exception& e) {}
}
