cmake_minimum_required(VERSION 3.2)

set(APP app)

project(SystemsArchitecture)

find_package(Poco REQUIRED COMPONENTS Foundation Util Net XML JSON Crypto NetSSL Data JWT)

if(NOT ${Poco_FOUND})
    message(FATAL_ERROR "Poco C++ Libraries not found.")
endif()

SET(CMAKE_CXX_STANDARD 17)
SET(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(${APP} main.cpp 
               config/config.cpp 
               database/database.cpp
               database/user.cpp
               user_server/user_request_handler/user_handler.cpp
               user_server/user_request_handler_factory/user_handler_factory.cpp
               user_server/user_server_application.cpp)

target_link_libraries(${APP} PRIVATE 
                      ${Poco_LIBRARIES}
                      "PocoDataPostgreSQL")