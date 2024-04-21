#include "message_handler.h"
#include "../../../mongo_database/message.h"
#include "../../../mongo_database/mongo_database.h"

#include <Poco/Net/HTTPServerRequest.h>
#include <Poco/URI.h>
#include <Poco/RegularExpression.h>
#include <Poco/Net/HTMLForm.h>

#include <iostream>

void MessageHandler::handleRequest(Poco::Net::HTTPServerRequest &request, Poco::Net::HTTPServerResponse &response)
{   
    try
    {      
        Poco::URI uri(request.getURI());

        if (uri.getPath() == "/message"
            && request.getMethod() == Poco::Net::HTTPRequest::HTTP_POST){
            Poco::Dynamic::Var jsonParseResult = MessageHandler::_jsonParser.parse(request.stream());
            Poco::JSON::Object::Ptr json = jsonParseResult.extract<Poco::JSON::Object::Ptr>();

            std::string reason;
            if (CheckSaveData(json, reason)){
                database::Message message = database::Message::fromJSON(json);

                message.Save();

                response.setStatus(Poco::Net::HTTPResponse::HTTP_OK);
                response.setChunkedTransferEncoding(true);
                response.setContentType("application/json");
                std::ostream &ostr = response.send();
                ostr << message.get_id();
            }
            else
            {
                response.setStatus(Poco::Net::HTTPResponse::HTTPStatus::HTTP_BAD_REQUEST);
                response.setChunkedTransferEncoding(true);
                response.setContentType("application/json");
                Poco::JSON::Object::Ptr root = new Poco::JSON::Object();
                root->set("status", "400");
                root->set("detail", "Message information is missing or incomplete: reason");
                root->set("instance", "/message");
                std::ostream &ostr = response.send();
                Poco::JSON::Stringifier::stringify(root, ostr);
                return;
            }

            return;
        }

        else if (uri.getPath() == "/message"
            && request.getMethod() == Poco::Net::HTTPRequest::HTTP_GET){
            Poco::URI::QueryParameters query = uri.getQueryParameters();

            bool isIdProviuded = false;
            long id;
            for (std::pair<std::string, std::string> pair : query)
                if (pair.first == "id"){
                    id = std::stoi(pair.second);
                    isIdProviuded = true;
                }
            
            if (isIdProviuded){
                std::optional<database::Message> message = database::Message::GetMessageById(id);

                if (message){
                    response.setStatus(Poco::Net::HTTPResponse::HTTP_OK);
                    response.setChunkedTransferEncoding(true);
                    response.setContentType("application/json");
                    std::ostream &ostr = response.send();
                    Poco::JSON::Stringifier::stringify(message->toJSON(), ostr);
                }
                else
                {
                    response.setStatus(Poco::Net::HTTPResponse::HTTPStatus::HTTP_NOT_FOUND);
                    response.setChunkedTransferEncoding(true);
                    response.setContentType("application/json");
                    Poco::JSON::Object::Ptr root = new Poco::JSON::Object();
                    root->set("status", "404");
                    root->set("detail", "Messages not found");
                    root->set("instance", "/message");
                    std::ostream &ostr = response.send();
                    Poco::JSON::Stringifier::stringify(root, ostr);
                    return;
                }

                return;
            }
            else
            {
                std::vector<database::Message> messages = database::Message::GetAllMessages();
                
                if (messages.size() == 0){
                    response.setStatus(Poco::Net::HTTPResponse::HTTPStatus::HTTP_NOT_FOUND);
                    response.setChunkedTransferEncoding(true);
                    response.setContentType("application/json");
                    Poco::JSON::Object::Ptr root = new Poco::JSON::Object();
                    root->set("status", "404");
                    root->set("detail", "Messages not found");
                    root->set("instance", "/message");
                    std::ostream &ostr = response.send();
                    Poco::JSON::Stringifier::stringify(root, ostr);
                    return;
                }

                Poco::JSON::Array jsonMessageArray;
                for (database::Message messages : messages){
                    jsonMessageArray.add(messages.toJSON());
                }

                response.setStatus(Poco::Net::HTTPResponse::HTTP_OK);
                response.setChunkedTransferEncoding(true);
                response.setContentType("application/json");
                std::ostream &ostr = response.send();

                Poco::JSON::Stringifier::stringify(jsonMessageArray, ostr); 
            }

            return;
        }

        else if (uri.getPath() == "/message/user"
                 && request.getMethod() == Poco::Net::HTTPRequest::HTTP_GET){
            Poco::URI::QueryParameters query = uri.getQueryParameters();

            bool isUserIdProviuded = false;
            long userId;
            for (std::pair<std::string, std::string> pair : query)
                if (pair.first == "id"){
                    userId = std::stoi(pair.second);
                    isUserIdProviuded = true;
                }

            if (isUserIdProviuded){
                std::vector<database::Message> messages = database::Message::GetUserMessages(userId);

                if (messages.size() == 0){
                    response.setStatus(Poco::Net::HTTPResponse::HTTPStatus::HTTP_NOT_FOUND);
                    response.setChunkedTransferEncoding(true);
                    response.setContentType("application/json");
                    Poco::JSON::Object::Ptr root = new Poco::JSON::Object();
                    root->set("status", "404");
                    root->set("detail", "Messages not found");
                    root->set("instance", "/message/user");
                    std::ostream &ostr = response.send();
                    Poco::JSON::Stringifier::stringify(root, ostr);
                    return;
                }

                Poco::JSON::Array jsonUserArray;
                for (database::Message message : messages){
                    jsonUserArray.add(message.toJSON());
                }

                response.setStatus(Poco::Net::HTTPResponse::HTTP_OK);
                response.setChunkedTransferEncoding(true);
                response.setContentType("application/json");
                std::ostream &ostr = response.send();

                Poco::JSON::Stringifier::stringify(jsonUserArray, ostr);
            }
            else{
                response.setStatus(Poco::Net::HTTPResponse::HTTPStatus::HTTP_BAD_REQUEST);
                response.setChunkedTransferEncoding(true);
                response.setContentType("application/json");
                Poco::JSON::Object::Ptr root = new Poco::JSON::Object();
                root->set("status", "400");
                root->set("detail", "Query parametr <id> is missing");
                root->set("instance", "/message/user");
                std::ostream &ostr = response.send();
                Poco::JSON::Stringifier::stringify(root, ostr);
                return;
            }
        }

        else if (uri.getPath() == "/message/chat"
                 && request.getMethod() == Poco::Net::HTTPRequest::HTTP_GET){
            Poco::URI::QueryParameters query = uri.getQueryParameters();

            bool isFirstUserIdProvided = false, isSecondUserIdProvided = false;
            long firstUserId, secondUserId;
            for (std::pair<std::string, std::string> pair : query){
                if (pair.first == "first_user_id"){
                    isFirstUserIdProvided = true;
                    firstUserId = std::stoi(pair.second);
                }
                else if (pair.first == "second_user_id"){
                    isSecondUserIdProvided = true;
                    secondUserId = std::stoi(pair.second);
                }
            }

            std::string reason;
            if (CheckGetDialogData(isFirstUserIdProvided, isSecondUserIdProvided, reason)){
                std::vector<database::Message> messages = database::Message::GetDialogMessages(firstUserId, secondUserId);

                if (messages.size() == 0){
                    response.setStatus(Poco::Net::HTTPResponse::HTTPStatus::HTTP_NOT_FOUND);
                    response.setChunkedTransferEncoding(true);
                    response.setContentType("application/json");
                    Poco::JSON::Object::Ptr root = new Poco::JSON::Object();
                    root->set("status", "404");
                    root->set("detail", "Messages not found");
                    root->set("instance", "/message/chat");
                    std::ostream &ostr = response.send();
                    Poco::JSON::Stringifier::stringify(root, ostr);
                    return;
                }

                Poco::JSON::Array jsonUserArray;
                for (database::Message message : messages){
                    jsonUserArray.add(message.toJSON());
                }

                response.setStatus(Poco::Net::HTTPResponse::HTTP_OK);
                response.setChunkedTransferEncoding(true);
                response.setContentType("application/json");
                std::ostream &ostr = response.send();

                Poco::JSON::Stringifier::stringify(jsonUserArray, ostr);
            }
            else
            {
                response.setStatus(Poco::Net::HTTPResponse::HTTPStatus::HTTP_BAD_REQUEST);
                response.setChunkedTransferEncoding(true);
                response.setContentType("application/json");
                Poco::JSON::Object::Ptr root = new Poco::JSON::Object();
                root->set("status", "400");
                root->set("detail", "Dialoge query parametrs is missing or incomplete: " + reason);
                root->set("instance", "/message/chat");
                std::ostream &ostr = response.send();
                Poco::JSON::Stringifier::stringify(root, ostr);
                return;
            }

            return;
        }

        else if (uri.getPath() == "/message"
                 && request.getMethod() == Poco::Net::HTTPRequest::HTTP_PUT){
            Poco::URI::QueryParameters query = uri.getQueryParameters();

            bool isIdProvided = false;
            long id;
            std::string password;
            for (std::pair<std::string, std::string> pair : query){
                if (pair.first == "id"){
                    isIdProvided = true;
                    id = std::stoi(pair.second);
                }
            }
            
            Poco::Dynamic::Var json1 = MessageHandler::_jsonParser.parse(request.stream());
            Poco::JSON::Object::Ptr json = json1.extract<Poco::JSON::Object::Ptr>();

            std::string reason;
            if (CheckUpdateData(isIdProvided, json, reason)){
                std::string text_content = json->get("text_content");
                database::Message::UpdateMessage(id, text_content);

                response.setStatus(Poco::Net::HTTPResponse::HTTP_OK);
                response.setChunkedTransferEncoding(true);
                response.setContentType("application/json");
                std::ostream &ostr = response.send();
                ostr << id;
            }
            else{
                response.setStatus(Poco::Net::HTTPResponse::HTTPStatus::HTTP_BAD_REQUEST);
                response.setChunkedTransferEncoding(true);
                response.setContentType("application/json");
                Poco::JSON::Object::Ptr root = new Poco::JSON::Object();
                root->set("status", "400");
                root->set("detail", "Update data is missing ot incomplete: " + reason);
                root->set("instance", "/message");
                std::ostream &ostr = response.send();
                Poco::JSON::Stringifier::stringify(root, ostr);

                return;
            }

            return;
        }

        else if (uri.getPath() == "/message"
                 && request.getMethod() == Poco::Net::HTTPRequest::HTTP_DELETE){
            Poco::URI::QueryParameters query = uri.getQueryParameters();

            bool isIdProvided = false;
            long id;
            std::string password;
            for (std::pair<std::string, std::string> pair : query){
                if (pair.first == "id"){
                    isIdProvided = true;
                    id = std::stoi(pair.second);
                }
            }

            if (isIdProvided){
                database::Message::DeleteMessage(id);

                response.setStatus(Poco::Net::HTTPResponse::HTTP_OK);
                response.setChunkedTransferEncoding(true);
                response.setContentType("application/json");
                std::ostream &ostr = response.send();
                ostr << id;
            }
            else{
                response.setStatus(Poco::Net::HTTPResponse::HTTPStatus::HTTP_BAD_REQUEST);
                response.setChunkedTransferEncoding(true);
                response.setContentType("application/json");
                Poco::JSON::Object::Ptr root = new Poco::JSON::Object();
                root->set("status", "400");
                root->set("detail", "Query parametr <id> is missing");
                root->set("instance", "/message");
                std::ostream &ostr = response.send();
                Poco::JSON::Stringifier::stringify(root, ostr);

                return;
            }

            return;
        }

        else{
            response.setStatus(Poco::Net::HTTPResponse::HTTPStatus::HTTP_BAD_REQUEST);
            response.setChunkedTransferEncoding(true);
            response.setContentType("application/json");
            Poco::JSON::Object::Ptr root = new Poco::JSON::Object();
            root->set("status", "400");
            root->set("detail", "Invalid path");
            root->set("instance", "uri.getPath()");
            std::ostream &ostr = response.send();
            Poco::JSON::Stringifier::stringify(root, ostr);

            return;
        }
    }
    catch (const Poco::JSON::JSONException e){
        response.setStatus(Poco::Net::HTTPResponse::HTTPStatus::HTTP_BAD_REQUEST);
        response.setChunkedTransferEncoding(true);
        response.setContentType("application/json");
        Poco::JSON::Object::Ptr root = new Poco::JSON::Object();
        root->set("status", "400");
        root->set("detail", "Invalid JSON format");
        root->set("instance", "uri.getPath()"); 
        std::ostream &ostr = response.send();
        Poco::JSON::Stringifier::stringify(root, ostr);
    }
    catch (...)
    {
        response.setStatus(Poco::Net::HTTPResponse::HTTPStatus::HTTP_BAD_REQUEST);
        response.setChunkedTransferEncoding(true);
        response.setContentType("application/json");
        Poco::JSON::Object::Ptr root = new Poco::JSON::Object();
        root->set("status", "500");
        root->set("detail", "Unexpected error");
        root->set("instance", "uri.getPath()"); 
        std::ostream &ostr = response.send();
        Poco::JSON::Stringifier::stringify(root, ostr);        
    }
}

bool MessageHandler::CheckSaveData(const Poco::JSON::Object::Ptr &json, std::string &reason)
{
    reason = "";
    bool isOK = true;
    
    if (!(json->has("id"))){
        reason += R"(filed <id> is missing in JSON body data)";
        isOK = false;
    }
    if (!(json->has("sender_id"))){
        if (!reason.empty())
            reason += ", ";
        reason += R"(filed <sender_id> is missing in JSON body data)";
        isOK = false;
    }
    if (!(json->has("receiver_id"))){
        if (!reason.empty())
            reason += ", ";
        reason += R"(filed <receiver_id> is missing in JSON body data)";
        isOK = false;
    }
    if (!(json->has("text_content"))){
        if (!reason.empty())
            reason += ", ";
        reason += R"(filed <text_content> is missing in JSON body data)";
        isOK = false;
    }
    
    return isOK;
}

bool MessageHandler::CheckGetDialogData(const bool &isFirstUserIdProvided, const bool &isSecondUserIdProvided, std::string &reason){
    reason = "";
    bool isOK = true;
    
    if (!isFirstUserIdProvided){
        reason += R"(query parametr <first_user_id> is missing)";
        isOK = false;
    }
    if (!isSecondUserIdProvided){
        reason += R"(query parametr <second_user_id> is missing)";
        isOK = false;
    }
    
    return isOK;
}

bool MessageHandler::CheckUpdateData(const bool &isIdProvided, const Poco::JSON::Object::Ptr &json, std::string &reason)
{
    reason = "";
    bool isOK = true;
    
    if (!isIdProvided){
        reason += R"(query parametr <id> is missing)";
        isOK = false;
    }
    if (!(json->has("text_content"))){
        if (!reason.empty())
            reason += ", ";
        reason += R"(filed <text_content> is missing in JSON body data)";
        isOK = false;
    }
    
    return isOK;
}