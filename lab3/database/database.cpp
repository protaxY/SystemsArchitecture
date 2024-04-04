#include "database.h"
#include "../config/config.h"

#include <Poco/MongoDB

#include <Poco/Data/PostgreSQL/Connector.h>
#include <Poco/Data/PostgreSQL/PostgreSQLException.h>

namespace database{
    Database::Database() : database_mongo(Config::get().get_mongo_database())
    {
        _connection_string += "host=";
        _connection_string += Config::get().get_host();
        _connection_string += " user=";
        _connection_string += Config::get().get_login();
        _connection_string += " dbname=";
        _connection_string += Config::get().get_database();
        _connection_string += " port=";
        _connection_string += Config::get().get_port();
        _connection_string += " password=";
        _connection_string += Config::get().get_password();

        std::cout << "# Connecting to postgresql: " << _connection_string << std::endl;
        Poco::Data::PostgreSQL::Connector::registerConnector();
        _pool = std::make_unique<Poco::Data::SessionPool>(Poco::Data::PostgreSQL::Connector::KEY, _connection_string);

        std::cout << "# Connecting to mongodb: " << Config::get().get_mongo() << ":" << Config::get().get_mongo_port()  << std::endl;
        connection_mongo.connect(Config::get().get_mongo(), atoi(Config::get().get_mongo_port().c_str()));
    }

    Database& Database::get(){
        static Database _instance;
        return _instance;
    }

    Poco::Data::Session Database::create_session(){
        return Poco::Data::Session(_pool->get());
    }
}