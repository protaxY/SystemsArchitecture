cmake_minimum_required(VERSION 3.2)

set(MESSAGE_APP app)

project(SystemsArchitecture)

find_package(Poco REQUIRED COMPONENTS Foundation Util Net XML JSON Crypto NetSSL Data JWT)

if(NOT ${Poco_FOUND})
    message(FATAL_ERROR "Poco C++ Libraries not found.")
endif()

SET(CMAKE_CXX_STANDARD 17)
SET(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(${MESSAGE_APP} main.cpp 
               ../config/config.cpp 
               ../mongo_database/mongo_database.cpp
               ../mongo_database/message.cpp
               ../auth_helper/auth_helper.cpp
               message_server/message_request_handler/message_handler.cpp
               message_server/message_request_handler_factory/message_handler_factory.cpp
               message_server/message_server_application.cpp)

target_link_libraries(${MESSAGE_APP} PRIVATE 
                      ${Poco_LIBRARIES}
                      "PocoMongoDB")