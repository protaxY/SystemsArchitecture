#include "user_handler.h"
#include "../../../database/user.h"
#include "../../../auth_helper/auth_helper.h"

#include <Poco/Net/HTTPServerRequest.h>
#include <Poco/URI.h>
#include <Poco/RegularExpression.h>
#include <Poco/Net/HTMLForm.h>

#include <iostream>

void MessageHandler::handleRequest(Poco::Net::HTTPServerRequest &request, Poco::Net::HTTPServerResponse &response)
{
    Poco::URI uri(request.getURI());
    
    try
    {             
        bool hasCredentials = false;
        bool isAuthorized = false;
        std::string login, password;

        std::string scheme;
        std::string info;
        try{
            request.getCredentials(scheme, info);
            hasCredentials = true;
        }
        catch (...){}

        if (hasCredentials){
            if (scheme == "Basic")
            {
                try{
                    AuthHelper::DecodeIdentity(info, login, password);
                    isAuthorized = true;
                }
                catch (...){
                    response.setStatus(Poco::Net::HTTPResponse::HTTPStatus::HTTP_UNAUTHORIZED);
                    response.setContentType("application/json");
                    Poco::JSON::Object::Ptr root = new Poco::JSON::Object();
                    root->set("type", "/errors/unauthorized");
                    root->set("title", "Internal exception");
                    root->set("status", "401");
                    root->set("detail", "failed to parse login and password, credentials must be in <login:password> format");
                    root->set("instance", "/user");
                    std::ostream &ostr = response.send();
                    Poco::JSON::Stringifier::stringify(root, ostr);
                    return;
                }
            }
            else{
                response.setStatus(Poco::Net::HTTPResponse::HTTPStatus::HTTP_UNAUTHORIZED);
                response.setContentType("application/json");
                Poco::JSON::Object::Ptr root = new Poco::JSON::Object();
                root->set("type", "/errors/unauthorized");
                root->set("title", "Internal exception");
                root->set("status", "401");
                root->set("detail", "authorization scheme must be <Basic>");
                root->set("instance", "/user");
                std::ostream &ostr = response.send();
                Poco::JSON::Stringifier::stringify(root, ostr);
                return;
            }
        }

        // создать нового пользователя
        if (uri.getPath() == "/user"
            && request.getMethod() == Poco::Net::HTTPRequest::HTTP_POST){
            
            Poco::Dynamic::Var json1 = MessageHandler::_jsonParser.parse(request.stream());
            Poco::JSON::Object::Ptr json = json1.extract<Poco::JSON::Object::Ptr>();

            std::string reason;
            if (CheckSaveData(json, reason)){
                database::User user = database::User::fromJSON(json);
                std::string previousLogin = json->getValue<std::string>("login");
                std::optional<database::User> previousUser = database::User::SearchByLogin(previousLogin);
                if (!previousUser){
                    user.Save();
                    response.setStatus(Poco::Net::HTTPResponse::HTTP_OK);
                    response.setContentType("application/json");
                    std::ostream &ostr = response.send();
                    ostr << user.get_id();
                    return;
                }
                else
                {
                    response.setStatus(Poco::Net::HTTPResponse::HTTPStatus::HTTP_BAD_REQUEST);
                    response.setContentType("application/json");
                    Poco::JSON::Object::Ptr root = new Poco::JSON::Object();
                    root->set("status", "400");
                    root->set("detail", "user with this login already exists");
                    root->set("instance", "/user");
                    std::ostream &ostr = response.send();
                    Poco::JSON::Stringifier::stringify(root, ostr);
                    return;
                }

                return;
            }
            else
            {
                response.setStatus(Poco::Net::HTTPResponse::HTTPStatus::HTTP_BAD_REQUEST);
                response.setContentType("application/json");
                Poco::JSON::Object::Ptr root = new Poco::JSON::Object();
                root->set("status", "400");
                root->set("detail", "user information is missing or incomplete: " + reason);
                root->set("instance", "/user");
                std::ostream &ostr = response.send();
                Poco::JSON::Stringifier::stringify(root, ostr);
                return;
            }

            return;
        }

        // получить пользователя по логину или получить всех пользователей
        else if (uri.getPath() == "/user"
                 && request.getMethod() == Poco::Net::HTTPRequest::HTTP_GET){
            std::string path = uri.getPath();
            Poco::URI::QueryParameters query = uri.getQueryParameters();

            bool isLoginProviuded = false;
            std::string login;
            for (std::pair<std::string, std::string> pair : query)
                if (pair.first == "login"){
                    login = pair.second;
                    isLoginProviuded = true;
                }

            if (isLoginProviuded){
                std::optional<database::User> user = database::User::SearchByLogin(login);

                if (user){
                    response.setStatus(Poco::Net::HTTPResponse::HTTP_OK);
                    response.setContentType("application/json");
                    std::ostream &ostr = response.send();
                    Poco::JSON::Stringifier::stringify(user->toJSON(), ostr);
                }
                else
                {
                    response.setStatus(Poco::Net::HTTPResponse::HTTPStatus::HTTP_NOT_FOUND);
                    response.setContentType("application/json");
                    Poco::JSON::Object::Ptr root = new Poco::JSON::Object();
                    root->set("status", "404");
                    root->set("detail", "user not found");
                    root->set("instance", "/user");
                    std::ostream &ostr = response.send();
                    Poco::JSON::Stringifier::stringify(root, ostr);
                    return;
                }

                return;
            }
            else
            {
                std::vector<database::User> users = database::User::GetAllUsers();

                if (users.size() == 0){
                    response.setStatus(Poco::Net::HTTPResponse::HTTPStatus::HTTP_NOT_FOUND);
                    response.setContentType("application/json");
                    Poco::JSON::Object::Ptr root = new Poco::JSON::Object();
                    root->set("status", "404");
                    root->set("detail", "users not found");
                    root->set("instance", "/user/search");
                    std::ostream &ostr = response.send();
                    Poco::JSON::Stringifier::stringify(root, ostr);
                    return;
                }

                Poco::JSON::Array jsonUserArray;
                for (database::User user : users){
                    jsonUserArray.add(user.toJSON());
                }

                response.setStatus(Poco::Net::HTTPResponse::HTTP_OK);
                response.setContentType("application/json");
                std::ostream &ostr = response.send();

                Poco::JSON::Stringifier::stringify(jsonUserArray, ostr);
            }
        }

        // поиск по маске фамилии и имени
        else if (uri.getPath() == "/user/search"
                 && request.getMethod() == Poco::Net::HTTPRequest::HTTP_GET){
            Poco::URI::QueryParameters query = uri.getQueryParameters();

            bool isFirstNameProvided = false, isLastNameProvided = false;  
            std::string first_name, last_name;
            for (std::pair<std::string, std::string> pair : query){
                if (pair.first == "first_name"){
                    isFirstNameProvided = true;
                    first_name = pair.second;
                }
                else if (pair.first == "last_name"){
                    isLastNameProvided = true;
                    last_name = pair.second;
                }
            }

            std::string reason;
            if (CheckSearchByFirstLastName(isFirstNameProvided, isLastNameProvided, reason)){
                std::vector<database::User> users = database::User::SearchByFirstLastName(first_name, last_name);

                if (users.size() == 0){
                    response.setStatus(Poco::Net::HTTPResponse::HTTPStatus::HTTP_NOT_FOUND);
                    response.setContentType("application/json");
                    Poco::JSON::Object::Ptr root = new Poco::JSON::Object();
                    root->set("status", "404");
                    root->set("detail", "users not found");
                    root->set("instance", "/user/search");
                    std::ostream &ostr = response.send();
                    Poco::JSON::Stringifier::stringify(root, ostr);
                    return;
                }

                Poco::JSON::Array jsonUserArray;
                for (database::User user : users){
                    jsonUserArray.add(user.toJSON());
                }

                response.setStatus(Poco::Net::HTTPResponse::HTTP_OK);
                response.setContentType("application/json");
                std::ostream &ostr = response.send();

                Poco::JSON::Stringifier::stringify(jsonUserArray, ostr);
            }
            else
            {
                response.setStatus(Poco::Net::HTTPResponse::HTTPStatus::HTTP_BAD_REQUEST);
                response.setContentType("application/json");
                Poco::JSON::Object::Ptr root = new Poco::JSON::Object();
                root->set("status", "400");
                root->set("detail", reason);
                root->set("instance", "/user/search");
                std::ostream &ostr = response.send();
                Poco::JSON::Stringifier::stringify(root, ostr);
                return;
            }

            return;
        }
        
        // удалить пользователя по ID
        else if (uri.getPath() == "/user"
                 && request.getMethod() == Poco::Net::HTTPRequest::HTTP_DELETE){
            
            if (!isAuthorized){
                response.setStatus(Poco::Net::HTTPResponse::HTTPStatus::HTTP_UNAUTHORIZED);
                response.setContentType("application/json");
                Poco::JSON::Object::Ptr root = new Poco::JSON::Object();
                root->set("type", "/errors/unauthorized");
                root->set("title", "Internal exception");
                root->set("status", "401");
                root->set("detail", "login and password must be provided in credentials with <Basic> scheme");
                root->set("instance", "/user");
                std::ostream &ostr = response.send();
                Poco::JSON::Stringifier::stringify(root, ostr);
                return;
            }

            std::optional<long> deletedId = database::User::Delete(login, password);
            if (deletedId){
                response.setStatus(Poco::Net::HTTPResponse::HTTP_OK);
                response.setContentType("application/json");
                std::ostream &ostr = response.send();
                ostr << *deletedId;
            }
            else
            {
                response.setStatus(Poco::Net::HTTPResponse::HTTPStatus::HTTP_BAD_REQUEST);
                response.setContentType("application/json");
                Poco::JSON::Object::Ptr root = new Poco::JSON::Object();
                root->set("status", "400");
                root->set("detail", "login or password is incorrect");
                root->set("instance", "/user");
                std::ostream &ostr = response.send();
                Poco::JSON::Stringifier::stringify(root, ostr);
                return;
            }
        }

        // обновить информацию о пользователе
        else if (uri.getPath() == "/user"
                 && request.getMethod() == Poco::Net::HTTPRequest::HTTP_PUT){
            
            if (!isAuthorized){
                response.setStatus(Poco::Net::HTTPResponse::HTTPStatus::HTTP_UNAUTHORIZED);
                response.setContentType("application/json");
                Poco::JSON::Object::Ptr root = new Poco::JSON::Object();
                root->set("type", "/errors/unauthorized");
                root->set("title", "Internal exception");
                root->set("status", "401");
                root->set("detail", "login and password must be provided in credentials with <Basic> scheme");
                root->set("instance", "/user");
                std::ostream &ostr = response.send();
                Poco::JSON::Stringifier::stringify(root, ostr);
                return;
            }
            
            Poco::Dynamic::Var json1 = MessageHandler::_jsonParser.parse(request.stream());
            Poco::JSON::Object::Ptr json = json1.extract<Poco::JSON::Object::Ptr>();
            
            std::string reason;
            if (CheckUpdateData(json, reason)){
                bool isValidUpdateUser = true;
                std::string validateResult;
                std::string reason;
                
                if (json->has("first_name") 
                    && !database::User::IsNameValid(json->getValue<std::string>("first_name"), reason)){
                    isValidUpdateUser = false;
                    validateResult += reason;
                    validateResult += "\n";
                }
                if (json->has("last_name") 
                    && !database::User::IsNameValid(json->getValue<std::string>("last_name"), reason)){
                    isValidUpdateUser = false;
                    validateResult += reason;
                    validateResult += "\n";
                }
                if (json->has("email") 
                    && !database::User::IsEmailValid(json->getValue<std::string>("email"), reason)){
                    isValidUpdateUser = false;
                    validateResult += reason;
                    validateResult += "\n";
                }

                if (isValidUpdateUser){
                    std::optional<long> updatedId = database::User::Update(login, password, json);
                    
                    if (updatedId){
                        response.setStatus(Poco::Net::HTTPResponse::HTTP_OK);
                        response.setContentType("application/json");
                        std::ostream &ostr = response.send();
                        ostr << *updatedId;
                    }
                    else
                    {
                        response.setStatus(Poco::Net::HTTPResponse::HTTPStatus::HTTP_BAD_REQUEST);
                        response.setContentType("application/json");
                        Poco::JSON::Object::Ptr root = new Poco::JSON::Object();
                        root->set("status", "400");
                        root->set("detail", "login or password is incorrect");
                        root->set("instance", "/user");
                        std::ostream &ostr = response.send();
                        Poco::JSON::Stringifier::stringify(root, ostr);

                        return;
                    }

                    return;
                }
                else
                {
                    response.setStatus(Poco::Net::HTTPResponse::HTTPStatus::HTTP_BAD_REQUEST);
                    response.setContentType("application/json");
                    Poco::JSON::Object::Ptr root = new Poco::JSON::Object();
                    root->set("status", "400");
                    root->set("detail", reason);
                    root->set("instance", "/user");
                    std::ostream &ostr = response.send();
                    Poco::JSON::Stringifier::stringify(root, ostr);
                    return;
                }
            }
            else
            {
                response.setStatus(Poco::Net::HTTPResponse::HTTPStatus::HTTP_BAD_REQUEST);
                response.setContentType("application/json");
                Poco::JSON::Object::Ptr root = new Poco::JSON::Object();
                root->set("status", "400");
                root->set("detail", reason);
                root->set("instance", "/user");
                std::ostream &ostr = response.send();
                Poco::JSON::Stringifier::stringify(root, ostr);

                return;
            }
        }

        // аутентификация
        else if (uri.getPath() == "/user/auth"
            && request.getMethod() == Poco::Net::HTTPRequest::HTTP_GET){

            std::string scheme;
            std::string info;
            try{
                request.getCredentials(scheme, info);
            }
            catch (...){
                response.setStatus(Poco::Net::HTTPResponse::HTTPStatus::HTTP_UNAUTHORIZED);
                response.setContentType("application/json");
                Poco::JSON::Object::Ptr root = new Poco::JSON::Object();
                root->set("type", "/errors/unauthorized");
                root->set("title", "Internal exception");
                root->set("status", "401");
                root->set("detail", "credentials are not provided");
                root->set("instance", "/user/auth");
                std::ostream &ostr = response.send();
                Poco::JSON::Stringifier::stringify(root, ostr);
                return; 
            }

            std::string login, password;
            if (scheme == "Basic")
            {
                try{
                    AuthHelper::DecodeIdentity(info, login, password);
                }
                catch (...){
                    response.setStatus(Poco::Net::HTTPResponse::HTTPStatus::HTTP_UNAUTHORIZED);
                    response.setContentType("application/json");
                    Poco::JSON::Object::Ptr root = new Poco::JSON::Object();
                    root->set("type", "/errors/unauthorized");
                    root->set("title", "Internal exception");
                    root->set("status", "401");
                    root->set("detail", "failed to parse login and password, credentials must be in <login:password> format");
                    root->set("instance", "/user/auth");
                    std::ostream &ostr = response.send();
                    Poco::JSON::Stringifier::stringify(root, ostr);
                    return;
                }
                
                if (std::optional<long> id = database::User::Auth(login, password)){
                    std::string token = AuthHelper::GenerateToken(*id, login);

                    response.setStatus(Poco::Net::HTTPResponse::HTTP_OK);
                    response.setContentType("application/token");
                    std::ostream &ostr = response.send();
                    ostr << token;
                    ostr.flush();
                    return;
                }
                else
                {
                    response.setStatus(Poco::Net::HTTPResponse::HTTPStatus::HTTP_UNAUTHORIZED);
                    response.setContentType("application/json");
                    Poco::JSON::Object::Ptr root = new Poco::JSON::Object();
                    root->set("type", "/errors/unauthorized");
                    root->set("title", "Internal exception");
                    root->set("status", "401");
                    root->set("detail", "login or password in incorrect");
                    root->set("instance", "/user/auth");
                    std::ostream &ostr = response.send();
                    Poco::JSON::Stringifier::stringify(root, ostr);
                    return;
                }
            }

            response.setStatus(Poco::Net::HTTPResponse::HTTPStatus::HTTP_UNAUTHORIZED);
            response.setContentType("application/json");
            Poco::JSON::Object::Ptr root = new Poco::JSON::Object();
            root->set("type", "/errors/unauthorized");
            root->set("title", "Internal exception");
            root->set("status", "401");
            root->set("detail", "authorization scheme must be <Basic>");
            root->set("instance", "/user/auth");
            std::ostream &ostr = response.send();
            Poco::JSON::Stringifier::stringify(root, ostr);
            return;
        }

        else
        {
            response.setStatus(Poco::Net::HTTPResponse::HTTPStatus::HTTP_BAD_REQUEST);
            response.setContentType("application/json");
            Poco::JSON::Object::Ptr root = new Poco::JSON::Object();
            root->set("status", "400");
            root->set("detail", "invalid path");
            root->set("instance", uri.getPath());
            std::ostream &ostr = response.send();
            Poco::JSON::Stringifier::stringify(root, ostr);

            return;
        }

        return;
    }
    catch (const Poco::JSON::JSONException e){
        response.setStatus(Poco::Net::HTTPResponse::HTTPStatus::HTTP_BAD_REQUEST);
        response.setContentType("application/json");
        Poco::JSON::Object::Ptr root = new Poco::JSON::Object();
        root->set("status", "400");
        root->set("detail", "invalid JSON format");
        root->set("instance", uri.getPath()); 
        std::ostream &ostr = response.send();
        Poco::JSON::Stringifier::stringify(root, ostr);
    }
    catch (...)
    {
        response.setStatus(Poco::Net::HTTPResponse::HTTPStatus::HTTP_BAD_REQUEST);
        response.setContentType("application/json");
        Poco::JSON::Object::Ptr root = new Poco::JSON::Object();
        root->set("status", "500");
        root->set("detail", "unexpected error");
        root->set("instance", uri.getPath()); 
        std::ostream &ostr = response.send();
        Poco::JSON::Stringifier::stringify(root, ostr);        
    }
}

bool MessageHandler::CheckSaveData(const Poco::JSON::Object::Ptr &json, std::string &reason)
{
    reason = "";
    std::string currentReason;
    bool isOK = true;
    
    if (!(json->has("first_name"))){
        reason += "filed <first_name> is missing in JSON body data";
        isOK = false;
    }
    else if (!database::User::IsNameValid(json->getValue<std::string>("first_name"), currentReason)){
        if (!reason.empty())
            reason += ", ";
        reason += currentReason;
        isOK = false;
    }

    if (!(json->has("last_name"))){
        if (!reason.empty())
            reason += ", ";
        reason += "filed <last_name> is missing in JSON body data";
        isOK = false;
    }
    else if (!database::User::IsNameValid(json->getValue<std::string>("last_name"), currentReason)){
        if (!reason.empty())
            reason += ", ";
        reason += currentReason;
        isOK = false;
    } 

    if (!(json->has("email"))){
        if (!reason.empty())
            reason += ", ";
        reason += "filed <email> is missing in JSON body data";
        isOK = false;
    }
    else if (!database::User::IsNameValid(json->getValue<std::string>("email"), currentReason)){
        if (!reason.empty())
            reason += ", ";
        reason += currentReason;
        isOK = false;
    } 

    if (!(json->has("title"))){
        if (!reason.empty())
            reason += ", ";
        reason += "filed <title> is missing in JSON body data";
        isOK = false;
    }
    if (!(json->has("login"))){
        if (!reason.empty())
            reason += ", ";
        reason += "filed <login> is missing in JSON body data";
        isOK = false;
    }
    if (!(json->has("password"))){
        if (!reason.empty())
            reason += ", ";
        reason += "filed <password> is missing in JSON body data";
        isOK = false;
    }
    
    return isOK;
}

bool MessageHandler::CheckSearchByFirstLastName(const bool &isFirstNameProvided, const bool &isLastNameProvided, std::string &reason){
    reason = "";
    bool isOK = true;
    
    if (!isFirstNameProvided){
        reason += "query parametr <first_name> is not provided";
        isOK = false;
    }
    if (!isLastNameProvided){
        if (!reason.empty())
            reason += ", ";
        reason += "query parametr <last_name> is not provided";
        isOK = false;
    }
    
    return isOK;
}

bool MessageHandler::CheckUpdateData(const Poco::JSON::Object::Ptr &json, std::string &reason)
{
    reason = "";
    bool isOK = true;
    
    if (!(json->has("first_name") || json->has("last_name") || json->has("email") || json->has("title") || json->has("password"))){
        reason += "JSON body data must contain at least one of these fields: <first_name>, <last_name>, <email>, <title>, <password>";
        isOK = false;
    }

    return isOK;
}