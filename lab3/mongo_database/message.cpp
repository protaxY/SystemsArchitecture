#include "message.h"
#include "mongo_database.h"

#include <Poco/MongoDB/Document.h>
#include <Poco/MongoDB/Element.h>
#include <Poco/MongoDB/ObjectId.h>
#include <Poco/DateTimeFormatter.h>
#include <Poco/DateTimeParser.h>

namespace database
{
    const std::string Message::collectionName = "messages";
    const std::string Message::timeFormat = "%Y %b %dd %H:%M:%S.%i %Z";
    const int Message::timeZoneDifferential = 0;
    
    Message Message::fromJSON(const Poco::JSON::Object::Ptr &json)
    {
        Message message;

        message.id() = json->getValue<long>("id");
        message.sender_id() = json->getValue<long>("sender_id");
        message.receiver_id() = json->getValue<long>("receiver_id");
        message.text_content() = json->getValue<std::string>("text_content");
        message.send_time() = Poco::DateTime();

        return message;
    }

    Poco::JSON::Object::Ptr Message::toJSON()
    {
        Poco::JSON::Object::Ptr root = new Poco::JSON::Object();

        root->set("id", _id);
        root->set("sender_id", _sender_id);
        root->set("receiver_id", _receiver_id);
        root->set("text_content", _text_content);
        root->set("send_time", _send_time);

        return root;
    }

    void Message::Save()
    {
        Poco::MongoDB::Document document;
        document.add<long>("id", id());
        document.add<long>("sender_id", sender_id());
        document.add<long>("receiver_id", receiver_id());
        document.add<std::string>("text_content", text_content());
        document.add<std::string>("send_time", Poco::DateTimeFormatter::format(get_send_time(), timeFormat));

        MongoDatabase::get().saveDocument(collectionName, document);
    }

    std::optional<Message> Message::GetMessageById(long &id)
    {
        Poco::MongoDB::Document document;
        document.add<long>("id", id);

        Poco::MongoDB::Document::Vector responseDocuments = MongoDatabase::get().getDocuments(collectionName, document);
        Poco::MongoDB::Document responseDocument;
        if (!responseDocuments.empty())
            responseDocument = *(responseDocuments[0]);
        else
            return {};
       
        Message message;
        message.id() = responseDocument.get<long>("id");
        message.sender_id() = responseDocument.get<long>("sender_id");
        message.receiver_id() = responseDocument.get<long>("receiver_id");
        message.text_content() = responseDocument.get<std::string>("text_content");

        const std::string send_time = responseDocument.get<std::string>("send_time");
        int timeZoneDifferential = timeZoneDifferential;
        message.send_time() = Poco::DateTimeParser::parse(timeFormat, send_time, timeZoneDifferential);

        return message;
    }

    std::vector<Message> Message::GetAllMessages()
    {
        Poco::MongoDB::Document document;

        Poco::MongoDB::Document::Vector responseDocuments = MongoDatabase::get().getDocuments(collectionName, document);
        
        std::vector<Message> messages;
        for (Poco::MongoDB::Document::Ptr document : responseDocuments){
            Message message;
            message.id() = document->get<long>("id");
            message.sender_id() = document->get<long>("sender_id");
            message.receiver_id() = document->get<long>("receiver_id");
            message.text_content() = document->get<std::string>("text_content");

            const std::string send_time = document->get<std::string>("send_time");
            int timeZoneDifferential = timeZoneDifferential;
            message.send_time() = Poco::DateTimeParser::parse(timeFormat, send_time, timeZoneDifferential);
            
            messages.push_back(message);
        }

        return messages;
    }

    std::vector<Message> Message::GetUserMessages(long &userId)
    {
        // Poco::MongoDB::Document document;
        
        
        // document.add<long>("sender_id", userId);

        // Poco::MongoDB::Document::Vector responseDocuments = MongoDatabase::get().getDocuments(collectionName, document);
    }

    void Message::UpdateMessage(long &id, Message &updateMessage)
    {
        Poco::MongoDB::Document document;
        
        document.add("sender_id", updateMessage.get_sender_id());
        document.add("receiver_id", updateMessage.get_receiver_id());
        document.add("text_content", updateMessage.get_text_content());

        MongoDatabase::get().updateDocument(collectionName, document);
    }

    void Message::DeleteMessage(long &id)
    {
        Poco::MongoDB::Document document;
        document.add("id", id);

        MongoDatabase::get().deleteDocument(collectionName, document);
    }

    long &Message::id()
    {
        return _id;
    }
    long &Message::sender_id()
    {
        return _sender_id;
    }
    long &Message::receiver_id()
    {
        return _receiver_id;
    }
    std::string &Message::text_content()
    {
        return _text_content;
    }
    Poco::DateTime &Message::send_time()
    {
        return _send_time;
    }

    const long &Message::get_id() const
    {
        return _id;
    }
    const long &Message::get_sender_id() const
    {
        return _sender_id;
    }
    const long &Message::get_receiver_id() const
    {
        return _receiver_id;
    }
    const std::string &Message::get_text_content() const
    {
        return _text_content;
    }
    const Poco::DateTime &Message::get_send_time() const
    {
        return _send_time;
    }
} // namespace database
