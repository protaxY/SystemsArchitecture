#include "../config/config.h"
#include "mongo_database.h"

namespace database{
    const std::string MongoDatabase::_messagesCollectionName = "messages";
    const std::string MongoDatabase::_postsCollectionName = "posts";
    
    MongoDatabase::MongoDatabase() : _database(Config::get().get_mongo_database())
    {
        std::cout << "# Connecting to mongodb: " << Config::get().get_mongo() << ":" << Config::get().get_mongo_port()  << std::endl;
        _connection.connect(Config::get().get_mongo(), atoi(Config::get().get_mongo_port().c_str()));
        MongoDatabase::CreateCollection(_messagesCollectionName);
        MongoDatabase::CreateCollection(_postsCollectionName);
    }

    void MongoDatabase::CreateCollection(const std::string &name)
    {
        Poco::SharedPtr<Poco::MongoDB::QueryRequest> createRequest = _database.createCommand();
        createRequest->selector().add("create", "messages3");
        _connection.sendRequest(*createRequest);
    }

    MongoDatabase &MongoDatabase::get()
    {
        static MongoDatabase _instance;
        return _instance;
    }

    Poco::Data::Session MongoDatabase::create_session()
    {
        return Poco::Data::Session(_pool->get());
    }

    void MongoDatabase::saveDocument(const std::string &collection, Poco::MongoDB::Document &document)
    {
        try
        {
            Poco::SharedPtr<Poco::MongoDB::InsertRequest> insertRequest = _database.createInsertRequest(collection);
            Poco::MongoDB::Document &insertDocument = insertRequest->addNewDocument();
            insertDocument = document;

            Poco::MongoDB::ResponseMessage response;
            _connection.sendRequest(*insertRequest, response);
        }
        catch (std::exception &ex)
        {
            std::cout << "mongodb exception: " << ex.what() << std::endl;
            std::string lastError = _database.getLastError(_connection);
            if (!lastError.empty())
                std::cout << "mongodb Last Error: " << lastError << std::endl;
        }
    }

    void MongoDatabase::updateDocument(const std::string &collection, Poco::MongoDB::Document &document)
    {
        try
        {
            Poco::SharedPtr<Poco::MongoDB::UpdateRequest> updateRequest = _database.createUpdateRequest(collection);
            Poco::MongoDB::Document &updateDocument = updateRequest->update();
            updateDocument = document;
            _connection.sendRequest(*updateRequest);
        }
        catch (std::exception &ex)
        {
            std::cout << "mongodb exception: " << ex.what() << std::endl;
            std::string lastError = _database.getLastError(_connection);
            if (!lastError.empty())
                std::cout << "mongodb Last Error: " << lastError << std::endl;
        }
    }

    Poco::MongoDB::Document::Vector MongoDatabase::getDocuments(const std::string &collection, Poco::MongoDB::Document &document)
    {
        try
        {
            // Poco::SharedPtr<Poco::MongoDB::QueryRequest> queryRequest = _database.createQueryRequest(collection);



            Poco::SharedPtr<Poco::MongoDB::QueryRequest> queryRequest = _database.createQueryRequest("messages");
            Poco::MongoDB::Document &queryDocument = queryRequest->selector();
            queryDocument = document;
            Poco::MongoDB::ResponseMessage response;
            _connection.sendRequest(*queryRequest, response);

            std::string a = document.toString();

            std::vector<std::string> result;
            for (auto doc : response.documents())
                result.push_back(doc->toString());

            return response.documents();
        }
        catch (std::exception &ex)
        {
            std::cout << "mongodb exception: " << ex.what() << std::endl;
            std::string lastError = _database.getLastError(_connection);
            if (!lastError.empty())
                std::cout << "mongodb Last Error: " << lastError << std::endl;
        }
    }

    long MongoDatabase::countDocuments(const std::string &collection, Poco::MongoDB::Document &document)
    {
        try
        {
            Poco::SharedPtr<Poco::MongoDB::QueryRequest> countRequest = _database.createCountRequest(collection);
            Poco::MongoDB::Document &countDocument = countRequest->selector();
            countDocument = document;
            
            Poco::MongoDB::ResponseMessage response;
            _connection.sendRequest(*countRequest, response);

            if ( response.hasDocuments() )
                return response.documents()[0]->getInteger("n");
        }
        catch (std::exception &ex)
        {
            std::cout << "mongodb exception: " << ex.what() << std::endl;
            std::string lastError = _database.getLastError(_connection);
            if (!lastError.empty())
                std::cout << "mongodb Last Error: " << lastError << std::endl;
        }

        return -1;
    }

    void MongoDatabase::deleteDocument(const std::string &collection, Poco::MongoDB::Document &document)
    {
        try
        {
            Poco::SharedPtr<Poco::MongoDB::DeleteRequest> deleteRequest = _database.createDeleteRequest(collection);
            Poco::MongoDB::Document &deleteDocument = deleteRequest->selector();
            deleteDocument = document;
            _connection.sendRequest(*deleteRequest);
        }
        catch (std::exception &ex)
        {
            std::cout << "mongodb exception: " << ex.what() << std::endl;
            std::string lastError = _database.getLastError(_connection);
            if (!lastError.empty())
                std::cout << "mongodb Last Error: " << lastError << std::endl;
        }
    }
}