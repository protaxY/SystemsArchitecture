#include "user_handler.h"
#include "../../database/user.h"

#include <Poco/Net/HTTPServerRequest.h>
#include <Poco/URI.h>
#include <Poco/RegularExpression.h>
#include <Poco/Net/HTMLForm.h>

#include <iostream>

const Poco::RegularExpression UserHandler::_userLoginRegEx("\\/user\\/[^\\/]+$");

std::string UserHandler::getUserLoginFromRegEx(std::string &match)
{
    return match.substr(6, match.length());
}

void UserHandler::handleRequest(Poco::Net::HTTPServerRequest &request, Poco::Net::HTTPServerResponse &response)
{
    try
    {      
        Poco::URI uri(request.getURI());

        // создать нового пользователя
        if (uri.getPath() == "/user"
            && request.getMethod() == Poco::Net::HTTPRequest::HTTP_POST){
            
            Poco::Dynamic::Var json1 = UserHandler::_jsonParser.parse(request.stream());
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
                        response.setChunkedTransferEncoding(true);
                        response.setContentType("application/json");
                        std::ostream &ostr = response.send();
                        ostr << user.get_id();
                    }
                    else
                    {
                        response.setStatus(Poco::Net::HTTPResponse::HTTPStatus::HTTP_BAD_REQUEST);
                        response.setChunkedTransferEncoding(true);
                        response.setContentType("application/json");
                        Poco::JSON::Object::Ptr root = new Poco::JSON::Object();
                        root->set("type", "/errors/bad_request");
                        root->set("title", "Internal exception");
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
                    response.setStatus(Poco::Net::HTTPResponse::HTTP_NOT_FOUND);
                    std::ostream &ostr = response.send();
                    ostr << validateResult;
                    response.send();

                    return;
                }
            }
            else
            {
                response.setStatus(Poco::Net::HTTPResponse::HTTPStatus::HTTP_BAD_REQUEST);
                response.setChunkedTransferEncoding(true);
                response.setContentType("application/json");
                Poco::JSON::Object::Ptr root = new Poco::JSON::Object();
                root->set("type", "/errors/bad_request");
                root->set("title", "Internal exception");
                root->set("status", "400");
                root->set("detail", "user information is missing or incomplete");
                root->set("instance", "/user");
                std::ostream &ostr = response.send();
                Poco::JSON::Stringifier::stringify(root, ostr);
                return;
            }

            std::cout << "4" << std::endl;
            return;
        }

        // получить пользователя по логину
        else if (_userLoginRegEx.match(uri.getPath())
                 && request.getMethod() == Poco::Net::HTTPRequest::HTTP_GET){
            std::string path = uri.getPath();
            std::string login = getUserLoginFromRegEx(path);
            std::optional<database::User> user = database::User::SearchByLogin(login);

            if (user){
                response.setStatus(Poco::Net::HTTPResponse::HTTP_OK);
                response.setChunkedTransferEncoding(true);
                response.setContentType("application/json");
                std::ostream &ostr = response.send();
                Poco::JSON::Stringifier::stringify(user->toJSON(), ostr);
            }
            else
            {
                response.setStatus(Poco::Net::HTTPResponse::HTTPStatus::HTTP_NOT_FOUND);
                response.setChunkedTransferEncoding(true);
                response.setContentType("application/json");
                Poco::JSON::Object::Ptr root = new Poco::JSON::Object();
                root->set("type", "/errors/not_found");
                root->set("title", "Internal exception");
                root->set("status", "404");
                root->set("detail", "user ot found");
                root->set("instance", "/user");
                std::ostream &ostr = response.send();
                Poco::JSON::Stringifier::stringify(root, ostr);
                return;
            }

            std::cout << "2" << std::endl;
            return;
        }

        // поиск по маске фамилии и имени
        else if (uri.getPath() == "/user/search"
                 && request.getMethod() == Poco::Net::HTTPRequest::HTTP_GET){
            Poco::URI::QueryParameters query = uri.getQueryParameters();

            std::string first_name, last_name;
            for (std::pair<std::string, std::string> pair : query){
                if (pair.first == "first_name"){
                    first_name = pair.second;
                }
                else if (pair.first == "last_name"){
                    last_name = pair.second;
                }
            }

            if (!first_name.empty() && last_name.empty()){
                std::vector<database::User> users = database::User::SearchByFirstLastName(first_name, last_name);

                Poco::JSON::Array jsonUserArray;
                for (database::User user : users){
                    jsonUserArray.add(user.toJSON());
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
                root->set("type", "/errors/bad_request");
                root->set("title", "Internal exception");
                root->set("status", "400");
                root->set("detail", "first name or last name mask is missing");
                root->set("instance", "/user/search");
                std::ostream &ostr = response.send();
                Poco::JSON::Stringifier::stringify(root, ostr);
                return;
            }

            std::cout << "3" << std::endl;
            return;
        }
        
        // удалить пользователя по ID
        else if (_userLoginRegEx.match(uri.getPath())
                 && request.getMethod() == Poco::Net::HTTPRequest::HTTP_DELETE){
            std::string path = uri.getPath();
            std::string login = getUserLoginFromRegEx(path);
            std::optional<long> deletedId = database::User::Delete(login);

            if (deletedId){
                response.setStatus(Poco::Net::HTTPResponse::HTTP_OK);
                response.setChunkedTransferEncoding(true);
                response.setContentType("application/json");
                std::ostream &ostr = response.send();
                ostr << *deletedId;
            }
            else
            {
                response.setStatus(Poco::Net::HTTPResponse::HTTPStatus::HTTP_NOT_FOUND);
                response.setChunkedTransferEncoding(true);
                response.setContentType("application/json");
                Poco::JSON::Object::Ptr root = new Poco::JSON::Object();
                root->set("type", "/errors/not_found");
                root->set("title", "Internal exception");
                root->set("status", "404");
                root->set("detail", "user ot found or password is incorrect");
                root->set("instance", "/user");
                std::ostream &ostr = response.send();
                Poco::JSON::Stringifier::stringify(root, ostr);
                return;
            }
        }

        // обновить информацию о пользователе
        else if (_userLoginRegEx.match(uri.getPath())
                 && request.getMethod() == Poco::Net::HTTPRequest::HTTP_PUT){
            std::string path = uri.getPath();
            std::string login = getUserLoginFromRegEx(path);

            Poco::Dynamic::Var json1 = UserHandler::_jsonParser.parse(request.stream());
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
                    std::string login = json->getValue<std::string>("login");
                    std::optional<long> updatedId = database::User::Update(login, json);
                    
                    if (updatedId){
                        response.setStatus(Poco::Net::HTTPResponse::HTTP_OK);
                        response.setChunkedTransferEncoding(true);
                        response.setContentType("application/json");
                        std::ostream &ostr = response.send();
                        ostr << *updatedId;
                    }
                    else
                    {
                        response.setStatus(Poco::Net::HTTPResponse::HTTPStatus::HTTP_NOT_FOUND);
                        response.setChunkedTransferEncoding(true);
                        response.setContentType("application/json");
                        Poco::JSON::Object::Ptr root = new Poco::JSON::Object();
                        root->set("type", "/errors/bad_request");
                        root->set("title", "Internal exception");
                        root->set("status", "404");
                        root->set("detail", "user not found or password is incorrect");
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
                response.setChunkedTransferEncoding(true);
                response.setContentType("application/json");
                Poco::JSON::Object::Ptr root = new Poco::JSON::Object();
                root->set("type", "/errors/bad_request");
                root->set("title", "Internal exception");
                root->set("status", "400");
                root->set("detail", "user information is missing or incomplete");
                root->set("instance", "/user");
                std::ostream &ostr = response.send();
                Poco::JSON::Stringifier::stringify(root, ostr);

                return;
            }
        }
        else
        {
            response.setStatus(Poco::Net::HTTPResponse::HTTPStatus::HTTP_BAD_REQUEST);
            response.setChunkedTransferEncoding(true);
            response.setContentType("application/json");
            Poco::JSON::Object::Ptr root = new Poco::JSON::Object();
            root->set("type", "/errors/bad_request");
            root->set("title", "Internal exception");
            root->set("status", "400");
            root->set("detail", "invalid path");
            root->set("instance", "uri.getPath()");
            std::ostream &ostr = response.send();
            Poco::JSON::Stringifier::stringify(root, ostr);

            return;
        }

        return;
    }
    catch (const Poco::JSON::JSONException e){
        response.setStatus(Poco::Net::HTTPResponse::HTTPStatus::HTTP_BAD_REQUEST);
        response.setChunkedTransferEncoding(true);
        response.setContentType("application/json");
        Poco::JSON::Object::Ptr root = new Poco::JSON::Object();
        root->set("type", "/errors/bad_request");
        root->set("title", "Internal exception");
        root->set("status", "400");
        root->set("detail", "invalid JSON");
        root->set("instance", "uri.getPath()"); 
        std::ostream &ostr = response.send();
        Poco::JSON::Stringifier::stringify(root, ostr);
    }
    catch (const std::exception& e)
    {
        std::cout << e.what() << std::endl;
    }
    catch (...)
    {
        std::cerr << "something wrong" << std::endl;
    }
}