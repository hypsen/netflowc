cmake_minimum_required(VERSION 3.2)

find_package(GTest REQUIRED)
find_package(Threads REQUIRED)
find_package(Boost 1.70.0 REQUIRED system)
find_package(Boost COMPONENTS program_options REQUIRED)

add_executable(netflowc netflowc.cpp NFCollector.cpp Logger.cpp ConsoleLogger.cpp FileLogger.cpp ClickHouseLogger.cpp utils.cpp)

target_link_libraries(netflowc
    boost_filesystem
    boost_system
    pthread
    boost_program_options
    ${CLICKHOUSE_BINARY_DIR}/clickhouse/libclickhouse-cpp-lib-static.a
    ${CLICKHOUSE_BINARY_DIR}/contrib/cityhash/libcityhash-lib.a
    ${CLICKHOUSE_BINARY_DIR}/contrib/lz4/liblz4-lib.a
)

target_compile_options(netflowc PRIVATE -Wall -Wextra -Werror -O3 -std=c++17)

find_program(CMAKE_CXX_CPPCHECK NAMES cppcheck)
if (CMAKE_CXX_CPPCHECK)
    message(STATUS "CPPCHECK FOUND")
    list(
        APPEND CMAKE_CXX_CPPCHECK 
            "--enable=warning"
            "--inconclusive"
            "--force" 
            "--inline-suppr"
            "--verbose"
    )
endif()

install(TARGETS netflowc RUNTIME DESTINATION bin)

