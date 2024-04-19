cmake_minimum_required(VERSION 3.2)

set(POST_APP app)

project(SystemsArchitecture)

find_package(Poco REQUIRED COMPONENTS Foundation Util Net XML JSON Crypto NetSSL Data JWT)

if(NOT ${Poco_FOUND})
    message(FATAL_ERROR "Poco C++ Libraries not found.")
endif()

SET(CMAKE_CXX_STANDARD 17)
SET(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(${POST_APP} main.cpp 
               ../config/config.cpp 
               ../mongo_database/mongo_database.cpp
               ../mongo_database/post.cpp
               ../auth_helper/auth_helper.cpp
               post_server/post_request_handler/post_handler.cpp
               post_server/post_request_handler_factory/post_handler_factory.cpp
               post_server/post_server_application.cpp)

target_link_libraries(${POST_APP} PRIVATE 
                      ${Poco_LIBRARIES}
                      "PocoMongoDB")