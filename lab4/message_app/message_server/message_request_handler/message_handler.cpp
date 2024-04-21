#include "message_handler.h"
#include "../../../mongo_database/message.h"
#include "../../../mongo_database/mongo_database.h"
#include "../../../auth_helper/auth_helper.h"

#include <Poco/Net/HTTPServerRequest.h>
#include <Poco/URI.h>
#include <Poco/RegularExpression.h>
#include <Poco/Net/HTMLForm.h>

#include <iostream>

void MessageHandler::handleRequest(Poco::Net::HTTPServerRequest &request, Poco::Net::HTTPServerResponse &response)
{   
    try
    {      
        long user_id = -1;
        std::string user_login;

        std::string scheme;
        std::string info;
        request.getCredentials(scheme, info);

        if (scheme == "Bearer"){
            std::string reason;
            if (!AuthHelper::ExtractPayload(info, user_id, user_login, reason)){
                response.setStatus(Poco::Net::HTTPResponse::HTTPStatus::HTTP_FORBIDDEN);
                response.setChunkedTransferEncoding(true);
                response.setContentType("application/json");
                Poco::JSON::Object::Ptr root = new Poco::JSON::Object();
                root->set("type", "/errors/not_authorized");
                root->set("title", "Internal exception");
                root->set("status", "403");
                root->set("detail", "JWT-token processing failed: " + reason);
                root->set("instance", "/post");
                std::ostream &ostr = response.send();
                Poco::JSON::Stringifier::stringify(root, ostr);
                return;                  
            }
        }
        
        Poco::URI uri(request.getURI());

        if (uri.getPath() == "/message"
            && request.getMethod() == Poco::Net::HTTPRequest::HTTP_POST){
            Poco::Dynamic::Var jsonParseResult = MessageHandler::_jsonParser.parse(request.stream());
            Poco::JSON::Object::Ptr json = jsonParseResult.extract<Poco::JSON::Object::Ptr>();

            json->set("sender_id", user_id);
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
                root->set("detail", "Message information is missing or incomplete: " + reason);
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
            
            if (id < 0){
                response.setStatus(Poco::Net::HTTPResponse::HTTPStatus::HTTP_BAD_REQUEST);
                response.setChunkedTransferEncoding(true);
                response.setContentType("application/json");
                Poco::JSON::Object::Ptr root = new Poco::JSON::Object();
                root->set("status", "400");
                root->set("detail", "query parametr <id> is invalid");
                root->set("instance", "/message");
                std::ostream &ostr = response.send();
                Poco::JSON::Stringifier::stringify(root, ostr);
                return;
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

            if (user_id <= 0){
                std::vector<database::Message> messages = database::Message::GetUserMessages(user_id);

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
                root->set("detail", "User id from JWT-token payload filed <id> is invalid");
                root->set("instance", "/message/user");
                std::ostream &ostr = response.send();
                Poco::JSON::Stringifier::stringify(root, ostr);
                return;
            }
        }

        else if (uri.getPath() == "/message/chat"
                 && request.getMethod() == Poco::Net::HTTPRequest::HTTP_GET){
            Poco::URI::QueryParameters query = uri.getQueryParameters();

            bool isSecondUserIdProvided = false;
            long secondUserId = -1;
            for (std::pair<std::string, std::string> pair : query){
                if (pair.first == "second_user_id"){
                    isSecondUserIdProvided = true;
                    secondUserId = std::stoi(pair.second);
                }
            }

            std::string reason;
            if (CheckGetDialogData(user_id, isSecondUserIdProvided, secondUserId, reason)){
                std::vector<database::Message> messages = database::Message::GetDialogMessages(user_id, secondUserId);

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
                root->set("detail", reason);
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
            long id = -1;
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
            if (CheckUpdateData(isIdProvided, id, json, reason)){
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
                root->set("detail", "Message update data is missing: " + reason);
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
            long id = -1;
            std::string password;
            for (std::pair<std::string, std::string> pair : query){
                if (pair.first == "id"){
                    isIdProvided = true;
                    id = std::stoi(pair.second);
                }
            }

            if (id < 0){
                response.setStatus(Poco::Net::HTTPResponse::HTTPStatus::HTTP_BAD_REQUEST);
                response.setChunkedTransferEncoding(true);
                response.setContentType("application/json");
                Poco::JSON::Object::Ptr root = new Poco::JSON::Object();
                root->set("status", "400");
                root->set("detail", "query parametr <id> is invalid");
                root->set("instance", "/message");
                std::ostream &ostr = response.send();
                Poco::JSON::Stringifier::stringify(root, ostr);
                return;
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
                root->set("detail", "Message id is missing");
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
        reason += R"(filed <sender_id> is missing in JWT-token payload data)";
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

bool CheckGetDialogData(const long &user_id, const bool &isSecondUserIdProvided, const long &secondUserId, std::string &reason){
    reason = "";
    bool isOK = true;
    
    if (user_id <= 0){
        reason += R"(User ID from JWT-token payload filed <id> is invalid)";
        isOK = false;
    }
    if (!isSecondUserIdProvided){
        if (!reason.empty())
            reason += ", ";
        reason += R"(query parametr <second_user_id> is missing)";
        isOK = false;
    }
    if (secondUserId <= 0){
        if (!reason.empty())
            reason += ", ";
        reason += R"(query parametr <second_user_id> is invalid)";
        isOK = false;
    }
    
    return isOK;
}

bool MessageHandler::CheckUpdateData(const bool &isIdProvided, const long &id, const Poco::JSON::Object::Ptr &json, std::string &reason)
{
    reason = "";
    bool isOK = true;
    
    if (!isIdProvided){
        reason += R"(query parametr <id> is missing)";
        isOK = false;
    }
    if (id < 0){
        if (!reason.empty())
            reason += ", ";
        reason += R"(query parametr <id> is invalid)";
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