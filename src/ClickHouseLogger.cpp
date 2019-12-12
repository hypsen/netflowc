#include "ClickHouseLogger.hpp"

#define CLICKHOUSE_RECONNECT_TIME 10
#define CLICKHOUSE_TABLE_ROTATE_PERIOD 60
#define CLICKHOUSE_BULK_INSERT_SIZE 500

void ClickHouseLogger::run()
{
    while (!taskDone_) {
        try {
            clickhouse::Client client(clickhouse::ClientOptions()
                                        .SetHost(host_)
                                        .SetPort(port_)
                                        .SetDefaultDatabase(db_)
                                        .SetUser(username_)
                                        .SetPassword(password_)
            );

            auto getLogDateTimeFromMSEC = [](uint64_t datetime) -> LogDateTime {
                auto seconds = static_cast<time_t>(datetime / 1000);
                auto t = *localtime(&seconds);

                return LogDateTime(t, CLICKHOUSE_TABLE_ROTATE_PERIOD);
            };

            auto getTableNameFromDateTime = [](LogDateTime& datetime) -> std::string {
                return std::string("netflow_" 
                                + std::to_string(datetime.year) + "_" 
                                + ((datetime.month) < 10 ? "0" : "") + std::to_string(datetime.month) + "_"
                                + ((datetime.day) < 10 ? "0" : "") + std::to_string(datetime.day) + "_"
                                + ((datetime.hour) < 10 ? "0" : "") + std::to_string(datetime.hour)
                        );

            };

            try {
                std::mutex wakeup_mutex;
                nfv9::DBRecord record;
                LogDateTime timeLast = LogDateTime();
                timeLast.period = CLICKHOUSE_TABLE_ROTATE_PERIOD;
                LogDateTime timeNew;
                timeNew = timeLast;
                std::string tableLast;
                std::string tableNew;

                while (!taskDone_) {
                    try {
                        std::unique_lock<std::mutex> ul(wakeup_mutex);
                        queueCV_.wait(ul);

                        bool queueEmpty = false;
                        while (!queueEmpty) {
                            clickhouse::Block block;
                            auto SENDER = std::make_shared<clickhouse::ColumnIPv4>();
                            auto RECEIVED_MSEC = std::make_shared<clickhouse::ColumnUInt64>();
                            auto IPV4_SRC_ADDR = std::make_shared<clickhouse::ColumnIPv4>();
                            auto IPV4_DST_ADDR = std::make_shared<clickhouse::ColumnIPv4>();
                            auto TOS = std::make_shared<clickhouse::ColumnUInt8>();
                            auto PROTOCOL = std::make_shared<clickhouse::ColumnUInt8>();
                            auto L4_SRC_PORT = std::make_shared<clickhouse::ColumnUInt16>();
                            auto L4_DST_PORT = std::make_shared<clickhouse::ColumnUInt16>();
                            auto ICMP_TYPE = std::make_shared<clickhouse::ColumnUInt16>();
                            auto INPUT_SNMP = std::make_shared<clickhouse::ColumnUInt32>();
                            auto SRC_VLAN = std::make_shared<clickhouse::ColumnUInt16>();
                            auto SRC_MASK = std::make_shared<clickhouse::ColumnUInt8>();
                            auto DST_MASK = std::make_shared<clickhouse::ColumnUInt8>();
                            auto SRC_AS = std::make_shared<clickhouse::ColumnUInt32>();
                            auto DST_AS = std::make_shared<clickhouse::ColumnUInt32>();
                            auto IPV4_NEXT_HOP = std::make_shared<clickhouse::ColumnIPv4>();
                            auto TCP_FLAGS = std::make_shared<clickhouse::ColumnUInt8>();
                            auto OUTPUT_SNMP = std::make_shared<clickhouse::ColumnUInt32>();
                            auto IN_BYTES = std::make_shared<clickhouse::ColumnUInt64>();
                            auto IN_PACKETS = std::make_shared<clickhouse::ColumnUInt64>();
                            auto FIRST_SWITCHED = std::make_shared<clickhouse::ColumnUInt32>();
                            auto LAST_SWITCHED = std::make_shared<clickhouse::ColumnUInt32>();
                            auto IP_PROTOCOL_VERSION = std::make_shared<clickhouse::ColumnUInt8>();
                            auto NF9_BGP_IPV4_NEXT_HOP = std::make_shared<clickhouse::ColumnIPv4>();
                            auto DIRECTION = std::make_shared<clickhouse::ColumnUInt8>();
                            auto NF_F_EVENT_TIME_MSEC = std::make_shared<clickhouse::ColumnUInt64>();
                            auto NF_F_XLATE_SRC_ADDR_IPV4 = std::make_shared<clickhouse::ColumnIPv4>();
                            auto NF_F_XLATE_DST_ADDR_IPV4 = std::make_shared<clickhouse::ColumnIPv4>();
                            auto NF_F_XLATE_SRC_PORT = std::make_shared<clickhouse::ColumnUInt16>();
                            auto NF_F_XLATE_DST_PORT = std::make_shared<clickhouse::ColumnUInt16>();
                            auto natEvent = std::make_shared<clickhouse::ColumnUInt8>();

                            std::size_t recordsToInsert = 0;
                            queueEmpty = true;

                            while (msgQueue_.pop(record)) {
                                timeNew = getLogDateTimeFromMSEC(record.RECEIVED_MSEC);
                                if (timeNew != timeLast) {
                                    tableNew = getTableNameFromDateTime(timeNew);

                                    client.Execute("CREATE TABLE IF NOT EXISTS " + tableNew + " ("
                                    + "SENDER IPv4," 
                                    + "RECEIVED_MSEC UInt64, "
                                    + "IPV4_SRC_ADDR IPv4, "
                                    + "IPV4_DST_ADDR IPv4, "
                                    + "TOS UInt8, "
                                    + "PROTOCOL UInt8, "
                                    + "L4_SRC_PORT UInt16, "
                                    + "L4_DST_PORT UInt16, "
                                    + "ICMP_TYPE UInt16, "
                                    + "INPUT_SNMP UInt32, "
                                    + "SRC_VLAN UInt16, "
                                    + "SRC_MASK UInt8, "
                                    + "DST_MASK UInt8, "
                                    + "SRC_AS UInt32, "
                                    + "DST_AS UInt32, "
                                    + "IPV4_NEXT_HOP IPv4, "
                                    + "TCP_FLAGS UInt8, "
                                    + "OUTPUT_SNMP UInt32, "
                                    + "IN_BYTES UInt64, "
                                    + "IN_PACKETS UInt64, "
                                    + "FIRST_SWITCHED UInt32, "
                                    + "LAST_SWITCHED UInt32, "
                                    + "IP_PROTOCOL_VERSION UInt8, "
                                    + "NF9_BGP_IPV4_NEXT_HOP IPv4, "
                                    + "DIRECTION UInt8, "
                                    + "NF_F_EVENT_TIME_MSEC UInt64, "
                                    + "NF_F_XLATE_SRC_ADDR_IPV4 IPv4, "
                                    + "NF_F_XLATE_DST_ADDR_IPV4 IPv4, "
                                    + "NF_F_XLATE_SRC_PORT UInt16, "
                                    + "NF_F_XLATE_DST_PORT UInt16, "
                                    + "natEvent UInt8"
                                    +") ENGINE = Log");
                                    break;
                                }
                                else {
                                    tableNew = tableLast;
                                }
                                
                                SENDER->Append(ntohl(static_cast<uint32_t>(record.SENDER)));
                                RECEIVED_MSEC->Append(record.RECEIVED_MSEC);
                                IPV4_SRC_ADDR->Append(ntohl(record.IPV4_SRC_ADDR));
                                IPV4_DST_ADDR->Append(ntohl(record.IPV4_DST_ADDR));
                                TOS->Append(record.TOS);
                                PROTOCOL->Append(record.PROTOCOL);
                                L4_SRC_PORT->Append(record.L4_SRC_PORT);
                                L4_DST_PORT->Append(record.L4_DST_PORT);
                                ICMP_TYPE->Append(record.ICMP_TYPE);
                                INPUT_SNMP->Append(record.INPUT_SNMP);
                                SRC_VLAN->Append(record.SRC_VLAN);
                                SRC_MASK->Append(record.SRC_MASK);
                                DST_MASK->Append(record.DST_MASK);
                                SRC_AS->Append(record.SRC_AS);
                                DST_AS->Append(record.DST_AS);
                                IPV4_NEXT_HOP->Append(ntohl(record.IPV4_NEXT_HOP));
                                TCP_FLAGS->Append(record.TCP_FLAGS);
                                OUTPUT_SNMP->Append(record.OUTPUT_SNMP);
                                IN_BYTES->Append(record.IN_BYTES);
                                IN_PACKETS->Append(record.IN_PACKETS);
                                FIRST_SWITCHED->Append(record.FIRST_SWITCHED);
                                LAST_SWITCHED->Append(record.LAST_SWITCHED);
                                IP_PROTOCOL_VERSION->Append(record.IP_PROTOCOL_VERSION);
                                NF9_BGP_IPV4_NEXT_HOP->Append(ntohl(record.NF9_BGP_IPV4_NEXT_HOP));
                                DIRECTION->Append(record.DIRECTION);
                                NF_F_EVENT_TIME_MSEC->Append(record.NF_F_EVENT_TIME_MSEC);
                                NF_F_XLATE_SRC_ADDR_IPV4->Append(ntohl(record.NF_F_XLATE_SRC_ADDR_IPV4));
                                NF_F_XLATE_DST_ADDR_IPV4->Append(ntohl(record.NF_F_XLATE_DST_ADDR_IPV4));
                                NF_F_XLATE_SRC_PORT->Append(record.NF_F_XLATE_SRC_PORT);
                                NF_F_XLATE_DST_PORT->Append(record.NF_F_XLATE_DST_PORT);
                                natEvent->Append(record.natEvent);

                                ++recordsToInsert;
                                if (recordsToInsert == CLICKHOUSE_BULK_INSERT_SIZE) {
                                    queueEmpty = false;
                                    break;
                                }
                            }

                            block.AppendColumn("SENDER", SENDER);
                            block.AppendColumn("RECEIVED_MSEC", RECEIVED_MSEC);
                            block.AppendColumn("IPV4_SRC_ADDR", IPV4_SRC_ADDR);
                            block.AppendColumn("IPV4_DST_ADDR", IPV4_DST_ADDR);
                            block.AppendColumn("TOS", TOS);
                            block.AppendColumn("PROTOCOL", PROTOCOL);
                            block.AppendColumn("L4_SRC_PORT", L4_SRC_PORT);
                            block.AppendColumn("L4_DST_PORT", L4_DST_PORT);
                            block.AppendColumn("ICMP_TYPE", ICMP_TYPE);
                            block.AppendColumn("INPUT_SNMP", INPUT_SNMP);
                            block.AppendColumn("SRC_VLAN", SRC_VLAN);
                            block.AppendColumn("SRC_MASK", SRC_MASK);
                            block.AppendColumn("DST_MASK", DST_MASK);
                            block.AppendColumn("SRC_AS", SRC_AS);
                            block.AppendColumn("DST_AS", DST_AS);
                            block.AppendColumn("IPV4_NEXT_HOP", IPV4_NEXT_HOP);
                            block.AppendColumn("TCP_FLAGS", TCP_FLAGS);
                            block.AppendColumn("OUTPUT_SNMP", OUTPUT_SNMP);
                            block.AppendColumn("IN_BYTES", IN_BYTES);
                            block.AppendColumn("IN_PACKETS", IN_PACKETS);
                            block.AppendColumn("FIRST_SWITCHED", FIRST_SWITCHED);
                            block.AppendColumn("LAST_SWITCHED", LAST_SWITCHED);
                            block.AppendColumn("IP_PROTOCOL_VERSION", IP_PROTOCOL_VERSION);
                            block.AppendColumn("NF9_BGP_IPV4_NEXT_HOP", NF9_BGP_IPV4_NEXT_HOP);
                            block.AppendColumn("DIRECTION", DIRECTION);
                            block.AppendColumn("NF_F_EVENT_TIME_MSEC", NF_F_EVENT_TIME_MSEC);
                            block.AppendColumn("NF_F_XLATE_SRC_ADDR_IPV4", NF_F_XLATE_SRC_ADDR_IPV4);
                            block.AppendColumn("NF_F_XLATE_DST_ADDR_IPV4", NF_F_XLATE_DST_ADDR_IPV4);
                            block.AppendColumn("NF_F_XLATE_SRC_PORT", NF_F_XLATE_SRC_PORT);
                            block.AppendColumn("NF_F_XLATE_DST_PORT", NF_F_XLATE_DST_PORT);
                            block.AppendColumn("natEvent", natEvent);

                            if (!tableLast.empty())
                                client.Insert(tableLast, block);

                            timeLast = timeNew;
                            tableLast = tableNew;
                        }
                    }
                    catch (...) {}
                }
            }
            catch (...) {}
        }
        catch (...) {}
        if (!taskDone_)
            std::this_thread::sleep_for(std::chrono::seconds(CLICKHOUSE_RECONNECT_TIME));
    }
}

bool ClickHouseLogger::push(const nfv9::DBRecord& msg)
{
    return msgQueue_.push(msg);
}