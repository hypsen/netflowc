#pragma once
#include <string>
#include <unordered_map>

namespace nfv9
{
    struct NetflowHeader
    {
        uint16_t versionNumber;
        uint16_t count;
        uint32_t sysuptime;
        uint32_t unixSecs;
        uint32_t sequenceNumber;
        uint32_t sourceId;
    } __attribute__((__packed__));

    struct FlowSetHeader {
        uint16_t flowSetId;
        uint16_t length;
    } __attribute__((__packed__));

    struct TemplateHeader {
        uint16_t templateId;
        uint16_t fieldCount;
    } __attribute__((__packed__));

    struct TemplateRecord {
        uint16_t fieldType; 
        uint16_t fieldLength;
    } __attribute__((__packed__));

    struct OptionsTemplateHeader {
        uint16_t templateId;
        uint16_t optionScopeLength;
        uint16_t optionLength;
    } __attribute__((__packed__));

    struct OptionsTemplateRecord {
        uint16_t fieldType; 
        uint16_t fieldLength;
    } __attribute__((__packed__));

    struct DBRecord {
        __uint128_t SENDER;
        uint64_t RECEIVED_MSEC;
        uint32_t IPV4_SRC_ADDR;
        uint32_t IPV4_DST_ADDR;
        uint8_t  TOS;
        uint8_t  PROTOCOL;
        uint16_t L4_SRC_PORT;
        uint16_t L4_DST_PORT;
        uint16_t ICMP_TYPE;
        uint32_t INPUT_SNMP;       // default uint16_t
        uint16_t SRC_VLAN;
        uint8_t  SRC_MASK;
        uint8_t  DST_MASK;
        uint32_t SRC_AS;
        uint32_t DST_AS;
        uint32_t IPV4_NEXT_HOP;
        uint8_t  TCP_FLAGS;
        uint32_t OUTPUT_SNMP;     // default uint16_t
        uint64_t IN_BYTES;        // default uint32_t
        uint64_t IN_PACKETS;      // default uint32_t
        uint32_t FIRST_SWITCHED;
        uint32_t LAST_SWITCHED;
        uint8_t  IP_PROTOCOL_VERSION;
        uint32_t NF9_BGP_IPV4_NEXT_HOP;
        uint8_t  DIRECTION;
        uint64_t NF_F_EVENT_TIME_MSEC;
        uint32_t NF_F_XLATE_SRC_ADDR_IPV4;
        uint32_t NF_F_XLATE_DST_ADDR_IPV4;
        uint16_t NF_F_XLATE_SRC_PORT;
        uint16_t NF_F_XLATE_DST_PORT;
        uint8_t  natEvent;
    };

    enum class Netflow_V9_Print_Function_Type {
        NUMBER,
        IPV4,
        IPV6,
        MAC,
        STRING,
        NATEVENT,
        DATETIME
    };

    const std::unordered_map<uint16_t, std::pair<std::string, Netflow_V9_Print_Function_Type>> NETFLOW_V9_TYPES =
    {
        // RFC3954
        {1,   {"IN_BYTES",                   Netflow_V9_Print_Function_Type::NUMBER}},
        {2,   {"IN_PACKETS",                 Netflow_V9_Print_Function_Type::NUMBER}},
        {3,   {"FLOWS",                      Netflow_V9_Print_Function_Type::NUMBER}},
        {4,   {"PROTOCOL",                   Netflow_V9_Print_Function_Type::NUMBER}},
        {5,   {"TOS",                        Netflow_V9_Print_Function_Type::NUMBER}},
        {6,   {"TCP_FLAGS",                  Netflow_V9_Print_Function_Type::NUMBER}},
        {7,   {"L4_SRC_PORT",                Netflow_V9_Print_Function_Type::NUMBER}},
        {8,   {"IPV4_SRC_ADDR",              Netflow_V9_Print_Function_Type::IPV4}},
        {9,   {"SRC_MASK",                   Netflow_V9_Print_Function_Type::NUMBER}},
        {10,  {"INPUT_SNMP",                 Netflow_V9_Print_Function_Type::NUMBER}},
        {11,  {"L4_DST_PORT",                Netflow_V9_Print_Function_Type::NUMBER}},
        {12,  {"IPV4_DST_ADDR",              Netflow_V9_Print_Function_Type::IPV4}},
        {13,  {"DST_MASK",                   Netflow_V9_Print_Function_Type::NUMBER}},
        {14,  {"OUTPUT_SNMP",                Netflow_V9_Print_Function_Type::NUMBER}},
        {15,  {"IPV4_NEXT_HOP",              Netflow_V9_Print_Function_Type::IPV4}},
        {16,  {"SRC_AS",                     Netflow_V9_Print_Function_Type::NUMBER}},
        {17,  {"DST_AS",                     Netflow_V9_Print_Function_Type::NUMBER}},
        {18,  {"NF9_BGP_IPV4_NEXT_HOP",      Netflow_V9_Print_Function_Type::IPV4}},
        {19,  {"MUL_DST_PKTS",               Netflow_V9_Print_Function_Type::NUMBER}},
        {20,  {"MUL_DST_BYTES",              Netflow_V9_Print_Function_Type::NUMBER}},
        {21,  {"LAST_SWITCHED",              Netflow_V9_Print_Function_Type::NUMBER}},
        {22,  {"FIRST_SWITCHED",             Netflow_V9_Print_Function_Type::NUMBER}},
        {23,  {"NF9_OUT_BYTES",              Netflow_V9_Print_Function_Type::NUMBER}},
        {24,  {"NF9_OUT_PKTS",               Netflow_V9_Print_Function_Type::NUMBER}},
        {27,  {"IPV6_SRC_ADDR",              Netflow_V9_Print_Function_Type::IPV6}},
        {28,  {"IPV6_DST_ADDR",              Netflow_V9_Print_Function_Type::IPV6}},
        {29,  {"IPV6_SRC_MASK",              Netflow_V9_Print_Function_Type::NUMBER}},
        {30,  {"IPV6_DST_MASK",              Netflow_V9_Print_Function_Type::NUMBER}},
        {31,  {"IPV6_FLOW_LABEL",            Netflow_V9_Print_Function_Type::NUMBER}},
        {32,  {"ICMP_TYPE",                  Netflow_V9_Print_Function_Type::NUMBER}},
        {34,  {"SAMPLING_INTERVAL",          Netflow_V9_Print_Function_Type::NUMBER}},
        {35,  {"SAMPLING_ALGORITHM",         Netflow_V9_Print_Function_Type::NUMBER}},
        {36,  {"FLOW_ACTIVE_TIMEOUT",        Netflow_V9_Print_Function_Type::NUMBER}},
        {37,  {"FLOW_INACTIVE_TIMEOUT",      Netflow_V9_Print_Function_Type::NUMBER}},
        {38,  {"ENGINE_TYPE",                Netflow_V9_Print_Function_Type::NUMBER}},
        {39,  {"ENGINE_ID",                  Netflow_V9_Print_Function_Type::NUMBER}},
        {40,  {"TOTAL_BYTES_EXP",            Netflow_V9_Print_Function_Type::NUMBER}},
        {41,  {"TOTAL_PKTS_EXP",             Netflow_V9_Print_Function_Type::NUMBER}},
        {42,  {"TOTAL_FLOWS_EXP",            Netflow_V9_Print_Function_Type::NUMBER}},
        {46,  {"MPLS_TOP_LABEL_TYPE",        Netflow_V9_Print_Function_Type::NUMBER}},
        {47,  {"MPLS_TOP_LABEL_IP_ADDR",     Netflow_V9_Print_Function_Type::NUMBER}},
        {48,  {"FLOW_SAMPLER_ID",            Netflow_V9_Print_Function_Type::NUMBER}},
        {49,  {"SAMPLER_MODE",               Netflow_V9_Print_Function_Type::NUMBER}},
        {50,  {"FLOW_SAMPLER_RANDOM_INTERVAL", Netflow_V9_Print_Function_Type::NUMBER}},
        {55,  {"DST_TOS",                    Netflow_V9_Print_Function_Type::NUMBER}},
        {56,  {"SRC_MAC",                    Netflow_V9_Print_Function_Type::MAC}},
        {57,  {"DST_MAC",                    Netflow_V9_Print_Function_Type::MAC}},
        {58,  {"SRC_VLAN",                   Netflow_V9_Print_Function_Type::NUMBER}},
        {59,  {"DST_VLAN",                   Netflow_V9_Print_Function_Type::NUMBER}},
        {60,  {"IP_PROTOCOL_VERSION",        Netflow_V9_Print_Function_Type::NUMBER}},
        {61,  {"DIRECTION",                  Netflow_V9_Print_Function_Type::NUMBER}},
        {62,  {"IPV6_NEXT_HOP",              Netflow_V9_Print_Function_Type::IPV6}},
        {63,  {"BPG_IPV6_NEXT_HOP",          Netflow_V9_Print_Function_Type::IPV6}},
        {64,  {"IPV6_OPTION_HEADERS",        Netflow_V9_Print_Function_Type::NUMBER}},

        {70,  {"MPLS_LABEL_1",               Netflow_V9_Print_Function_Type::NUMBER}},
        {71,  {"MPLS_LABEL_2",               Netflow_V9_Print_Function_Type::NUMBER}},
        {72,  {"MPLS_LABEL_3",               Netflow_V9_Print_Function_Type::NUMBER}},
        {73,  {"MPLS_LABEL_4",               Netflow_V9_Print_Function_Type::NUMBER}},
        {74,  {"MPLS_LABEL_5",               Netflow_V9_Print_Function_Type::NUMBER}},
        {75,  {"MPLS_LABEL_6",               Netflow_V9_Print_Function_Type::NUMBER}},
        {76,  {"MPLS_LABEL_7",               Netflow_V9_Print_Function_Type::NUMBER}},
        {77,  {"MPLS_LABEL_8",               Netflow_V9_Print_Function_Type::NUMBER}},
        {78,  {"MPLS_LABEL_9",               Netflow_V9_Print_Function_Type::NUMBER}},
        {79,  {"MPLS_LABEL_10",              Netflow_V9_Print_Function_Type::NUMBER}},

        // from https://www.cisco.com/en/US/technologies/tk648/tk362/technologies_white_paper09186a00800a3db9.html
        {80,  {"IN_DST_MAC",                  Netflow_V9_Print_Function_Type::MAC}},
        {81,  {"OUT_SRC_MAC",                 Netflow_V9_Print_Function_Type::MAC}},
        {82,  {"IF_NAME",                     Netflow_V9_Print_Function_Type::STRING}},
        {83,  {"IF_DESC",                     Netflow_V9_Print_Function_Type::STRING}},
        {84,  {"SAMPLER_NAME",                Netflow_V9_Print_Function_Type::STRING}},
        {85,  {"IN_PERMANENT_BYTES",          Netflow_V9_Print_Function_Type::NUMBER}},
        {86,  {"IN_PERMANENT_PKTS",           Netflow_V9_Print_Function_Type::NUMBER}},
        {88,  {"FRAGMENT_OFFSET",             Netflow_V9_Print_Function_Type::NUMBER}},
        {89,  {"FORWARDING_STATUS",           Netflow_V9_Print_Function_Type::NUMBER}},
        {90,  {"MPLS_PAL_RD",                 Netflow_V9_Print_Function_Type::NUMBER}},
        {91,  {"MPLS_PREFIX_LEN",             Netflow_V9_Print_Function_Type::NUMBER}},
        {92,  {"SRC_TRAFFIC_INDEX",           Netflow_V9_Print_Function_Type::NUMBER}},
        {93,  {"DST_TRAFFIC_INDEX",           Netflow_V9_Print_Function_Type::NUMBER}},
        {94,  {"APPLICATION_DESCRIPTION",     Netflow_V9_Print_Function_Type::STRING}},
        {95,  {"APPLICATION_TAG",             Netflow_V9_Print_Function_Type::NUMBER}},
        {96,  {"APPLICATION_NAME",            Netflow_V9_Print_Function_Type::STRING}},
        {98,  {"postipDiffServCodePoint",     Netflow_V9_Print_Function_Type::NUMBER}},
        {99,  {"replication_factor",          Netflow_V9_Print_Function_Type::NUMBER}},
        {100, {"DEPRECATED_1",                Netflow_V9_Print_Function_Type::NUMBER}},
        {102, {"layer2packetSectionOffset",   Netflow_V9_Print_Function_Type::NUMBER}},
        {103, {"layer2packetSectionSize",     Netflow_V9_Print_Function_Type::NUMBER}},
        {104, {"layer2packetSectionData",     Netflow_V9_Print_Function_Type::NUMBER}},

        // https://www.iana.org/assignments/ipfix/ipfix.xhtml
        {230, {"natEvent",                    Netflow_V9_Print_Function_Type::NATEVENT}},

        // Cisco NSEL extension
        // from https://www.cisco.com/c/en/us/td/docs/security/asa/special/netflow/guide/asa_netflow.html
        {148, {"NF_F_CONN_ID",                Netflow_V9_Print_Function_Type::NUMBER}},
        {152, {"NF_F_FLOW_CREATE_TIME_MSEC",  Netflow_V9_Print_Function_Type::NUMBER}},
        {176, {"NF_F_ICMP_TYPE",              Netflow_V9_Print_Function_Type::NUMBER}},
        {177, {"NF_F_ICMP_CODE",              Netflow_V9_Print_Function_Type::NUMBER}},
        {178, {"NF_F_ICMP_TYPE_IPV6",         Netflow_V9_Print_Function_Type::NUMBER}},
        {179, {"NF_F_ICMP_CODE_IPV6",         Netflow_V9_Print_Function_Type::NUMBER}},
        {225, {"NF_F_XLATE_SRC_ADDR_IPV4",    Netflow_V9_Print_Function_Type::IPV4}},
        {226, {"NF_F_XLATE_DST_ADDR_IPV4",    Netflow_V9_Print_Function_Type::IPV4}},
        {227, {"NF_F_XLATE_SRC_PORT",         Netflow_V9_Print_Function_Type::NUMBER}},
        {228, {"NF_F_XLATE_DST_PORT",         Netflow_V9_Print_Function_Type::NUMBER}},
        {231, {"NF_F_FWD_FLOW_DELTA_BYTES",   Netflow_V9_Print_Function_Type::NUMBER}},
        {232, {"NF_F_REV_FLOW_DELTA_BYTES",   Netflow_V9_Print_Function_Type::NUMBER}},
        {233, {"NF_F_FW_EVENT",               Netflow_V9_Print_Function_Type::NUMBER}},
        {281, {"NF_F_XLATE_SRC_ADDR_IPV6",    Netflow_V9_Print_Function_Type::IPV6}},
        {282, {"NF_F_XLATE_DST_ADDR_IPV6",    Netflow_V9_Print_Function_Type::IPV6}},
        {323, {"NF_F_EVENT_TIME_MSEC",        Netflow_V9_Print_Function_Type::NUMBER}},
        {33000, {"NF_F_INGRESS_ACL_ID",       Netflow_V9_Print_Function_Type::NUMBER}},
        {33001, {"NF_F_EGRESS_ACL_ID",        Netflow_V9_Print_Function_Type::NUMBER}},
        {33002, {"NF_F_FW_EXT_EVENT",         Netflow_V9_Print_Function_Type::NUMBER}},
        {40000, {"NF_F_USERNAME",             Netflow_V9_Print_Function_Type::STRING}}
    };

    enum class Nf_Server_Exception
    {
        UNKNOWN_TEMPLATE,
        END_OF_PACKET
    };

}