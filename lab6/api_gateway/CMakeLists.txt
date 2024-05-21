cmake_minimum_required(VERSION 3.2)

set(APP app)

project(SystemsArchitecture)

find_package(Poco REQUIRED COMPONENTS Foundation Util Net XML JSON Crypto NetSSL Data JWT)

if(NOT ${Poco_FOUND})
    message(FATAL_ERROR "Poco C++ Libraries not found.")
endif()

set (STD_CXX "c++17")
set (REDISCPP_FLAGS "-DREDISCPP_HEADER_ONLY=ON")
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -W -Wall -std=${STD_CXX} ${REDISCPP_FLAGS}")
set (CMAKE_CXX_FLAGS_RELEASE "-O3 -g0 -std=${STD_CXX} -Wall -DNDEBUG ${REDISCPP_FLAGS}")

SET(CMAKE_CXX_STANDARD 17)
SET(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(${APP} main.cpp 
               ../config/config.cpp
               ../cache/cache.cpp
               api_gateway_application.cpp
               api_gateway_request_handler_factory/api_gateway_factory.cpp
               api_gateway_request_handler/api_gateway_handler.cpp
               circuit_breaker/circuit_breaker.cpp)

target_link_libraries(${APP} PRIVATE
                      ${Poco_LIBRARIES})