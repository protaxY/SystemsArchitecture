#include "config.h"

Config::Config()
{
        _host = std::getenv("DB_HOST");
        _port = std::getenv("DB_PORT");
        _login = std::getenv("DB_LOGIN");
        _password = std::getenv("DB_PASSWORD");
        _database = std::getenv("DB_DATABASE");
        
        _mongo = std::getenv("MONGO_HOST");
        _mongo_port = std::getenv("MONGO_PORT");
        _mongo_database = std::getenv("MONGO_DATABASE");

        _cache_host = std::getenv("CACHE_HOST");
        _cache_port = std::getenv("CACHE_PORT");

        _user_port = std::getenv("USER_PORT");
        _message_port = std::getenv("MESSAGE_PORT");
        _post_port = std::getenv("POST_PORT");
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

const std::string &Config::get_port() const
{
    return _port;
}

const std::string &Config::get_host() const
{
    return _host;
}

const std::string &Config::get_login() const
{
    return _login;
}

const std::string &Config::get_password() const
{
    return _password;
}
const std::string &Config::get_database() const
{
    return _database;
}

std::string &Config::port()
{
    return _port;
}

std::string &Config::host()
{
    return _host;
}

std::string &Config::login()
{
    return _login;
}

std::string &Config::password()
{
    return _password;
}

std::string &Config::database()
{
    return _database;
}

const std::string &Config::get_cache_host() const
{
    return _cache_host;
}

const std::string &Config::get_cache_port() const
{
    return _cache_port;
}

const std::string &Config::get_user_port() const
{
    return _user_port;
}

const std::string &Config::get_message_port() const
{
    return _message_port;
}

const std::string &Config::get_post_port() const
{
    return _post_port;
}

Config &Config::get()
{
    static Config _instance;
    return _instance;
}