#ifndef DATABASE_H
#define DATABASE_H

#include <string>
#include <memory>

#include <Poco/Data/SessionPool.h>

namespace database{
    class Database{
        private:
            std::string _connection_string;
            std::unique_ptr<Poco::Data::SessionPool> _pool;
            Database();
        public:
            void CreateTableIfNotExists();
            static Database& get();
            Poco::Data::Session create_session();
    };
}
#endif // DATABASE_H