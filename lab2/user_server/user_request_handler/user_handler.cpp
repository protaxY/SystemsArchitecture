#include "user_handler.h"
#include "../../database/user.h"

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

        // создать нового пользователя
        if (uri.getPath() == "/user"
            && request.getMethod() == Poco::Net::HTTPRequest::HTTP_POST){
            
            Poco::Dynamic::Var json1 = MessageHandler::_jsonParser.parse(request.stream());
            Poco::JSON::Object::Ptr json = json1.extract<Poco::JSON::Object::Ptr>();
            
            if (json->has("first_name") && json->has("last_name") && json->has("email") && json->has("title") && json->has("login") && json->has("password")){
                
                bool isValidUser = true;
                std::string validateResult;
                std::string reason;

                if (!database::User::IsNameValid(json->getValue<std::string>("first_name"), reason)){
                    isValidUser = false;
                    validateResult += reason;
                    validateResult += "\n";
                }
                if (!database::User::IsNameValid(json->getValue<std::string>("last_name"), reason)){
                    isValidUser = false;
                    validateResult += reason;
                    validateResult += "\n";
                }
                if (!database::User::IsEmailValid(json->getValue<std::string>("email"), reason)){
                    isValidUser = false;
                    validateResult += reason;
                    validateResult += "\n";
                }

                if (isValidUser){
                    database::User user = database::User::fromJSON(json);
                    std::string login = json->getValue<std::string>("login");
                    std::optional<database::User> previousUser = database::User::SearchByLogin(login);
                    if (!previousUser){
                        user.Save();
                        response.setStatus(Poco::Net::HTTPResponse::HTTP_OK);
                        response.setContentType("application/json");
                        std::ostream &ostr = response.send();
                        ostr << user.get_id();
                    }
                    else
                    {
                        response.setStatus(Poco::Net::HTTPResponse::HTTPStatus::HTTP_BAD_REQUEST);
                        response.setContentType("application/json");
                        Poco::JSON::Object::Ptr root = new Poco::JSON::Object();
                        root->set("status", "400");
                        root->set("detail", "User with this login already exists");
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
                    root->set("detail", validateResult);
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
                root->set("detail", "User information is missing or incomplete");
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
                    root->set("detail", "User not found or login not provided");
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
                    root->set("detail", "Users not found");
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

            if (isFirstNameProvided && isLastNameProvided){
                std::vector<database::User> users = database::User::SearchByFirstLastName(first_name, last_name);

                if (users.size() == 0){
                    response.setStatus(Poco::Net::HTTPResponse::HTTPStatus::HTTP_NOT_FOUND);
                    response.setContentType("application/json");
                    Poco::JSON::Object::Ptr root = new Poco::JSON::Object();
                    root->set("status", "404");
                    root->set("detail", "Users not found");
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
                root->set("detail", "First name or last name mask is missing");
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
            std::string path = uri.getPath();
            std::string login;

            Poco::URI::QueryParameters query = uri.getQueryParameters();
            std::string password;
            for (std::pair<std::string, std::string> pair : query){
                if (pair.first == "login")
                    login = pair.second;
                else if (pair.first == "password")
                    password = pair.second;
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
            std::string path = uri.getPath();
            std::string login;

            Poco::URI::QueryParameters query = uri.getQueryParameters();
            std::string password;
            for (std::pair<std::string, std::string> pair : query){
                if (pair.first == "login")
                    login = pair.second;
                else if (pair.first == "password")
                    password = pair.second;
            }
            
            Poco::Dynamic::Var json1 = MessageHandler::_jsonParser.parse(request.stream());
            Poco::JSON::Object::Ptr json = json1.extract<Poco::JSON::Object::Ptr>();
            
            if (json->has("first_name") || json->has("last_name") || json->has("email") || json->has("title") || json->has("password")){
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
                    response.setStatus(Poco::Net::HTTPResponse::HTTP_BAD_REQUEST);
                    std::ostream &ostr = response.send();
                    ostr << validateResult;
                    response.send();

                    return;
                }
            }
            else
            {
                response.setStatus(Poco::Net::HTTPResponse::HTTPStatus::HTTP_BAD_REQUEST);
                response.setContentType("application/json");
                Poco::JSON::Object::Ptr root = new Poco::JSON::Object();
                root->set("status", "400");
                root->set("detail", "User information is missing or incomplete");
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
            root->set("detail", "Invalid path");
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
        root->set("detail", "Invalid JSON format");
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
        root->set("detail", "Unexpected error");
        root->set("instance", uri.getPath()); 
        std::ostream &ostr = response.send();
        Poco::JSON::Stringifier::stringify(root, ostr);        
    }
}