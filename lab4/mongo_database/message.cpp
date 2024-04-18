#include "message.h"
#include "mongo_database.h"

#include <Poco/MongoDB/Document.h>
#include <Poco/MongoDB/Element.h>
#include <Poco/MongoDB/ObjectId.h>
#include <Poco/DateTimeFormatter.h>
#include <Poco/DateTimeParser.h>
#include <Poco/MongoDB/Array.h>

namespace database
{
    const std::string Message::collectionName = "messages";
    const std::string Message::timeFormat = "%Y %b %dd %H:%M:%S %Z";
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
        document.add("id", id());
        document.add("sender_id", sender_id());
        document.add("receiver_id", receiver_id());
        document.add("text_content", text_content());
        document.add("send_time", Poco::DateTimeFormatter::format(get_send_time(), timeFormat));

        MongoDatabase::get().saveDocument(collectionName, document);
    }

    std::optional<Message> Message::GetMessageById(long &id)
    {
        Poco::MongoDB::Document document;
        document.add("id", id);

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
        Poco::MongoDB::Document document;
        
        Poco::MongoDB::Document::Ptr senderIdDocPtr = new Poco::MongoDB::Document;
        senderIdDocPtr->add("sender_id", userId);

        Poco::MongoDB::Document::Ptr receiverIdDocPtr = new Poco::MongoDB::Document;
        receiverIdDocPtr->add("receiver_id", userId);

        Poco::MongoDB::Array::Ptr matchFieldsArrPtr = new Poco::MongoDB::Array;
        matchFieldsArrPtr->add(senderIdDocPtr);
        matchFieldsArrPtr->add(receiverIdDocPtr);

        document.add("$and", matchFieldsArrPtr);

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

    std::vector<Message> Message::GetDialogMessages(long &firstUserId, long &secondUserId)
    {
        Poco::MongoDB::Document document;
        
        Poco::MongoDB::Document::Ptr senderIdDocPtr = new Poco::MongoDB::Document;
        senderIdDocPtr->add("sender_id", firstUserId);
        Poco::MongoDB::Document::Ptr receiverIdDocPtr = new Poco::MongoDB::Document;
        receiverIdDocPtr->add("receiver_id", secondUserId);

        Poco::MongoDB::Array::Ptr firstSecondArrPtr = new Poco::MongoDB::Array;
        firstSecondArrPtr->add(senderIdDocPtr);
        firstSecondArrPtr->add(receiverIdDocPtr);

        Poco::MongoDB::Document::Ptr andFirstSecondDocPtr = new Poco::MongoDB::Document;
        andFirstSecondDocPtr->add("$and", firstSecondArrPtr);


        Poco::MongoDB::Document::Ptr senderIdDocPtr2 = new Poco::MongoDB::Document;
        senderIdDocPtr2->add("sender_id", secondUserId);
        Poco::MongoDB::Document::Ptr receiverIdDocPtr2 = new Poco::MongoDB::Document;
        receiverIdDocPtr2->add("receiver_id", firstUserId);

        Poco::MongoDB::Array::Ptr secondFirstArrPtr = new Poco::MongoDB::Array;
        secondFirstArrPtr->add(senderIdDocPtr2);
        secondFirstArrPtr->add(receiverIdDocPtr2);

        Poco::MongoDB::Document::Ptr andSecondFirstDocPtr = new Poco::MongoDB::Document;
        andSecondFirstDocPtr->add("$and", secondFirstArrPtr);

        
        Poco::MongoDB::Array::Ptr orArrPtr = new Poco::MongoDB::Array;
        orArrPtr->add(andFirstSecondDocPtr);
        orArrPtr->add(andSecondFirstDocPtr);

        document.add("$or", orArrPtr);

        std::string a = document.toString();

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

    void Message::UpdateMessage(long &id, std::string &text_content)
    {
        Poco::MongoDB::Document selectorDocument;

        Poco::MongoDB::Document::Ptr setDocument = new Poco::MongoDB::Document();
        setDocument->add("text_content", text_content);
        Poco::MongoDB::Document updateDocument;
        updateDocument.add("$set", setDocument);

        MongoDatabase::get().updateDocument(collectionName, selectorDocument, updateDocument);
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
