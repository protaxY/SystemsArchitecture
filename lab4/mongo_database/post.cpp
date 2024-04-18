#include "post.h"
#include "mongo_database.h"

#include <Poco/MongoDB/Document.h>
#include <Poco/MongoDB/Element.h>
#include <Poco/MongoDB/ObjectId.h>
#include <Poco/DateTimeFormatter.h>
#include <Poco/DateTimeParser.h>
#include <Poco/MongoDB/Array.h>

namespace database
{
    const std::string Post::collectionName = "posts";
    const std::string Post::timeFormat = "%Y %b %dd %H:%M:%S %Z";
    const int Post::timeZoneDifferential = 0;
    
    Post Post::fromJSON(const Poco::JSON::Object::Ptr &json)
    {
        Post message;

        message.id() = json->getValue<long>("id");
        message.author_id() = json->getValue<long>("author_id");
        message.text_content() = json->getValue<std::string>("text_content");
        message.send_time() = Poco::DateTime();

        return message;
    }

    Poco::JSON::Object::Ptr Post::toJSON()
    {
        Poco::JSON::Object::Ptr root = new Poco::JSON::Object();

        root->set("id", _id);
        root->set("author_id", _author_id);
        root->set("text_content", _text_content);
        root->set("send_time", _send_time);

        return root;
    }

    void Post::Save()
    {
        Poco::MongoDB::Document document;
        document.add("id", id());
        document.add("author_id", author_id());
        document.add("text_content", text_content());
        document.add("send_time", Poco::DateTimeFormatter::format(get_send_time(), timeFormat));

        MongoDatabase::get().saveDocument(collectionName, document);
    }

    std::optional<Post> Post::GetPostById(long &id)
    {
        Poco::MongoDB::Document document;
        document.add("id", id);

        Poco::MongoDB::Document::Vector responseDocuments = MongoDatabase::get().getDocuments(collectionName, document);
        Poco::MongoDB::Document responseDocument;
        if (!responseDocuments.empty())
            responseDocument = *(responseDocuments[0]);
        else
            return {};
       
        Post message;
        message.id() = responseDocument.get<long>("id");
        message.author_id() = responseDocument.get<long>("author_id");
        message.text_content() = responseDocument.get<std::string>("text_content");

        const std::string send_time = responseDocument.get<std::string>("send_time");
        int timeZoneDifferential = timeZoneDifferential;
        message.send_time() = Poco::DateTimeParser::parse(timeFormat, send_time, timeZoneDifferential);

        return message;
    }

    std::vector<Post> Post::GetAllPosts()
    {
        Poco::MongoDB::Document document;

        Poco::MongoDB::Document::Vector responseDocuments = MongoDatabase::get().getDocuments(collectionName, document);
        
        std::vector<Post> messages;
        for (Poco::MongoDB::Document::Ptr document : responseDocuments){
            Post message;
            message.id() = document->get<long>("id");
            message.author_id() = document->get<long>("author_id");
            message.text_content() = document->get<std::string>("text_content");

            const std::string send_time = document->get<std::string>("send_time");
            int timeZoneDifferential = timeZoneDifferential;
            message.send_time() = Poco::DateTimeParser::parse(timeFormat, send_time, timeZoneDifferential);
            
            messages.push_back(message);
        }

        return messages;
    }

    std::vector<Post> Post::GetUserPosts(long &userId)
    {
        Poco::MongoDB::Document document;

        document.add("author_id", userId);

        Poco::MongoDB::Document::Vector responseDocuments = MongoDatabase::get().getDocuments(collectionName, document);

        std::vector<Post> messages;
        for (Poco::MongoDB::Document::Ptr document : responseDocuments){
            Post message;
            message.id() = document->get<long>("id");
            message.author_id() = document->get<long>("author_id");
            message.text_content() = document->get<std::string>("text_content");

            const std::string send_time = document->get<std::string>("send_time");
            int timeZoneDifferential = timeZoneDifferential;
            message.send_time() = Poco::DateTimeParser::parse(timeFormat, send_time, timeZoneDifferential);
            
            messages.push_back(message);
        }

        return messages;
    }

    void Post::UpdatePost(long &id, std::string &text_content)
    {
        Poco::MongoDB::Document selectorDocument;

        Poco::MongoDB::Document::Ptr setDocument = new Poco::MongoDB::Document();
        setDocument->add("text_content", text_content);
        Poco::MongoDB::Document updateDocument;
        updateDocument.add("$set", setDocument);

        MongoDatabase::get().updateDocument(collectionName, selectorDocument, updateDocument);
    }

    void Post::DeletePost(long &id)
    {
        Poco::MongoDB::Document document;
        document.add("id", id);

        MongoDatabase::get().deleteDocument(collectionName, document);
    }

    long &Post::id()
    {
        return _id;
    }
    long &Post::author_id()
    {
        return _author_id;
    }
    std::string &Post::text_content()
    {
        return _text_content;
    }
    Poco::DateTime &Post::send_time()
    {
        return _send_time;
    }

    const long &Post::get_id() const
    {
        return _id;
    }
    const long &Post::get_author_id() const
    {
        return _author_id;
    }
    const std::string &Post::get_text_content() const
    {
        return _text_content;
    }
    const Poco::DateTime &Post::get_send_time() const
    {
        return _send_time;
    }
} // namespace database
