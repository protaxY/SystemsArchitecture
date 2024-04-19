#include "config.h"

Config::Config()
{
        _mongo = std::getenv("MONGO_HOST");
        _mongo_port = std::getenv("MONGO_PORT");
        _mongo_database = std::getenv("MONGO_DATABASE");
}

std::string& Config::mongo(){
    return _mongo;
}
std::string& Config::mongo_port(){
    return _mongo_port;
}
std::string& Config::mongo_database(){
    return _mongo_database;
}

const std::string& Config::get_mongo() const{
    return _mongo;
}
const std::string& Config::get_mongo_port() const{
    return _mongo_port;
}
const std::string& Config::get_mongo_database() const{
    return _mongo_database;
}

Config &Config::get()
{
    static Config _instance;
    return _instance;
}