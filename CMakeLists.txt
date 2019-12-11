cmake_minimum_required(VERSION 3.2)

if ($ENV{TRAVIS_BUILD_NUMBER})
    project(netflowc VERSION 0.0.$ENV{TRAVIS_BUILD_NUMBER})
else ()
    project(netflowc VERSION 0.0.1)
endif ()

include(ExternalProject)

set(EXTERNAL_INSTALL_LOCATION ${CMAKE_CURRENT_SOURCE_DIR})

ExternalProject_Add(span
    SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/externals/span"
    GIT_REPOSITORY https://github.com/tcbrindle/span
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
    INSTALL_COMMAND ""
)

ExternalProject_Add(rapidjson
    SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/externals/rapidjson"
    GIT_REPOSITORY https://github.com/Tencent/rapidjson
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
    INSTALL_COMMAND ""
)

ExternalProject_Add(clickhouse-cpp
    SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/externals/clickhouse-cpp"
    GIT_REPOSITORY https://github.com/artpaul/clickhouse-cpp
    PATCH_COMMAND /usr/bin/patch ${CMAKE_CURRENT_SOURCE_DIR}/externals/clickhouse-cpp/cmake/cpp17.cmake ${CMAKE_CURRENT_SOURCE_DIR}/patch/cpp17.cmake.patch
    INSTALL_COMMAND ""
)

ExternalProject_Get_Property(clickhouse-cpp BINARY_DIR)
set(CLICKHOUSE_BINARY_DIR ${BINARY_DIR})

add_subdirectory(src)

set(CPACK_GENERATOR DEB)
set(CPACK_PACKAGE_VERSION_MAJOR "${PROJECT_VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${PROJECT_VERSION_MINOR}")
set(CPACK_PACKAGE_VERSION_PATCH "${PROJECT_VERSION_PATCH}")

set(CPACK_PACKAGE_CONTACT example@example.com)
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "netflowc")

include(CPack)
