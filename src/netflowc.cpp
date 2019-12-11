#include <iostream>
#include <vector>
#include <boost/program_options.hpp>
#include <filesystem>
#include <chrono>
#include <boost/asio.hpp>
#include "NFCollector.hpp"

namespace po = boost::program_options;
namespace fs = std::filesystem;

#define QUEUE_SIZE 100
#define DEFAULT_CLICKHOUSE_PORT 9000

int main(int argc, char* argv[])
try {
    po::options_description desc("Allowed options");
    desc.add_options()
            ("help,h", "list allowed commands")
            ("listen,l", po::value<std::string>(), "ip address to listen, default listen all interfaces and ip")
            ("port,r", po::value<uint16_t>(), "listen udp port")
            ("stdout,o", "write netflow logs to stdout")
            ("file,f", "write netflow logs to file")
            ("clickhouse,c", "write netflow logs to clickhouse")
            ("dir,d", po::value<std::string>(), "directory to write netflow logs")
            ("period,m", po::value<uint8_t>(), "file rotate period in minutes, allowed values - 1,5,10,15,20,30 or 60 minutes")
            ("dbhost,n", po::value<std::string>(), "clickhouse ip address")
            ("dbport,e", po::value<uint16_t>(), "clickhouse tcp port")
            ("database,b",po::value<std::string>(),"clickhouse database name")
            ("user,u", po::value<std::string>(), "username for clickhouse")
            ("password,p", po::value<std::string>(), "password for clickhouse")
            ("buffersize,s", po::value<uint16_t>(), "message buffer size, default = 100");

    po::variables_map vm;
    po::parsed_options parsed = po::command_line_parser(argc, argv).options(desc).allow_unregistered().run();
    po::store(parsed, vm);
    po::notify(vm);

    if (vm.count("help")) {
        std::cerr << desc << "\n";
        return 1;
    }

    uint16_t queueSize = QUEUE_SIZE;
    if (vm.count("buffersize")) {
        queueSize = vm["bufferSize"].as<uint16_t>();
        if (queueSize < QUEUE_SIZE)
            queueSize = QUEUE_SIZE;
    }

    uint16_t udpPort;
    if (vm.count("port")) {
        udpPort = vm["port"].as<uint16_t>();
    }
    else {
        std::cerr << "Specify udp port to listen.\n";
        return 1;
    }

    boost::asio::ip::udp::endpoint endpoint;        
    if (vm.count("listen"))
        endpoint = {boost::asio::ip::address_v4::from_string(vm["listen"].as<std::string>()), udpPort};
    else
        endpoint = {boost::asio::ip::udp::v4(), udpPort};

    boost::asio::io_context io_context;
    std::ios::sync_with_stdio(false);
    NFServer server(io_context,endpoint);

    if (vm.count("stdout"))
        server.addConsoleLogger(queueSize);

    fs::path fspath;
    if (vm.count("dir")) {
        fspath = fs::canonical(fs::path(vm["dir"].as<std::string>()));
        if (!fs::exists(fspath)) {
            std::cerr << "Directory does not exist.\n";
            return 1;
        }
        if (!fs::is_directory(fspath)) {
            std::cerr << "Path is not a directory.\n";
            return 1;
        }
    }

    uint8_t rotatePeriod = 15;
    if (vm.count("period")) {
        rotatePeriod = vm["period"].as<uint8_t>();
        switch (rotatePeriod) {
            case 1: case 5: case 10: case 15: case 30: case 60:
                break;
            default:
                std::cerr << "Wrong period.\n";
                return 1;
        }
    }

    if (vm.count("file")) {
        if (fspath.empty()) {
            std::cerr << "Specify directory.\n";
            return 1;
        }
        server.addFileLogger(fspath, rotatePeriod, queueSize);
    }

    boost::asio::ip::address_v4 dbhost;
    if (vm.count("dbhost")) {
        dbhost = boost::asio::ip::address_v4::from_string(vm["dbhost"].as<std::string>());
    }

    uint16_t dbport = DEFAULT_CLICKHOUSE_PORT;
    if (vm.count("dbport")) {
        dbport = vm["dbport"].as<uint16_t>();
    }

    std::string database;
    if (vm.count("database")) {
        database = vm["database"].as<std::string>();
    }

    std::string user;
    if (vm.count("user")) {
        user = vm["user"].as<std::string>();
    }

    std::string password;
    if (vm.count("password")) {
        password = vm["password"].as<std::string>();
    }

    if (vm.count("clickhouse")) {
        if (database.empty()) {
            std::cerr << "Specify database.\n";
            return 1;
        }
        server.addClickHouseLogger(dbhost.to_string(), dbport, database, user, password, queueSize);
    }

    if (server.getLoggersCount() == 0) {
        std::cerr << "Specify logging method.\n";
        return 1;
    }

    server.run();
    io_context.run();
    return 0;
}
catch (std::exception& e) {
    std::cerr << "Exception: " << e.what() << "\n";
    return 1;
}
catch (...) {
    return 1;
}
