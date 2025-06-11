cmake_minimum_required(VERSION 3.10.0)
project(ProcessMD VERSION 0.1.0 LANGUAGES C CXX)

find_package(OpenSSL REQUIRED)

# Path to the websocketpp library
set(WEBSOCKETPP_PATH /home/git_repos/websocketpp)

add_executable(ProcessMD main.cpp
                         MDConnector.cpp
                         simdjson.cpp)

target_link_libraries(ProcessMD PRIVATE OpenSSL::SSL OpenSSL::Crypto)
target_include_directories(ProcessMD PUBLIC
        ${WEBSOCKETPP_PATH}
    )

include(CTest)
enable_testing()

set_property(TARGET ProcessMD PROPERTY CXX_STANDARD 17)