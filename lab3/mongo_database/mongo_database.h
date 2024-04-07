#ifndef DATABASE_H
#define DATABASE_H

#include <string>
#include <memory>

#include <Poco/Data/SessionPool.h>
#include <Poco/MongoDB/Database.h>
#include <Poco/MongoDB/Document.h>

namespace database{
    class MongoDatabase{
        private:
            std::unique_ptr<Poco::Data::SessionPool> _pool;

            Poco::MongoDB::Database                  _database;
            Poco::MongoDB::Connection                _connection;
            static const std::string                 _messagesCollectionName;
            static const std::string                 _postsCollectionName;
            
            MongoDatabase();
            void CreateCollection(const std::string &name);
        public:
            static MongoDatabase& get();
            Poco::Data::Session create_session();
            void saveDocument(const std::string &collection, Poco::MongoDB::Document &document);
            void updateDocument(const std::string &collection, Poco::MongoDB::Document &document);
            Poco::MongoDB::Document::Vector getDocuments(const std::string &collection, Poco::MongoDB::Document &document);
            long countDocuments(const std::string& collection, Poco::MongoDB::Document &document);
            void deleteDocument(const std::string &collection, Poco::MongoDB::Document &document);
    };
}
#endif // DATABASE_H