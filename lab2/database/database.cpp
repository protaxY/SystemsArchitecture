#include "database.h"
#include "../config/config.h"

#include <Poco/Data/PostgreSQL/Connector.h>
#include <Poco/Data/PostgreSQL/PostgreSQLException.h>

namespace database{
    Database::Database(){
        _connection_string+="host=";
        _connection_string+=Config::get().get_host();
        _connection_string+=" user=";
        _connection_string+=Config::get().get_login();
        _connection_string+=" dbname=";
        _connection_string+=Config::get().get_database();
        _connection_string+=" port=";
        _connection_string+=Config::get().get_port();
        _connection_string+=" password=";
        _connection_string+=Config::get().get_password();

        std::cout << "Connection string:" << _connection_string << std::endl;
        Poco::Data::PostgreSQL::Connector::registerConnector();
        _pool = std::make_unique<Poco::Data::SessionPool>(Poco::Data::PostgreSQL::Connector::KEY, _connection_string);
    }

    void Database::CreateTableIfNotExists(){
        try
        {
            Poco::Data::Session session = database::Database::get().create_session();
            Poco::Data::Statement create_stmt(session);
            create_stmt << "CREATE TABLE IF NOT EXISTS users (id SERIAL,"
                        << "first_name VARCHAR(256) NOT NULL,"
                        << "last_name VARCHAR(256) NOT NULL,"
                        << "login VARCHAR(256) NOT NULL,"
                        << "password VARCHAR(256) NOT NULL,"
                        << "email VARCHAR(256) NULL,"
                        << "title VARCHAR(1024) NULL);",
            Poco::Data::Keywords::now;
        }
        catch (Poco::Data::PostgreSQL::PostgreSQLException &e)
        {
            std::cout << "connection:" << e.displayText() << std::endl;
            throw;
        }
        catch (Poco::Data::ConnectionFailedException &e)
        {
            std::cout << "connection:" << e.displayText() << std::endl;
            throw;
        }
    }

    Database& Database::get(){
        static Database _instance;
        return _instance;
    }

    Poco::Data::Session Database::create_session(){
        return Poco::Data::Session(_pool->get());
    }
}