#include "post_handler.h"
#include "../../../mongo_database/post.h"
#include "../../../mongo_database/mongo_database.h"
#include "../../../auth_helper/auth_helper.h"

#include <Poco/Net/HTTPServerRequest.h>
#include <Poco/URI.h>
#include <Poco/RegularExpression.h>
#include <Poco/Net/HTMLForm.h>

#include <iostream>

void PostHandler::handleRequest(Poco::Net::HTTPServerRequest &request, Poco::Net::HTTPServerResponse &response)
{   
    Poco::URI uri(request.getURI());
    
    try
    {      
        long user_id = -1;
        std::string user_login;

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
            root->set("instance", "/post");
            std::ostream &ostr = response.send();
            Poco::JSON::Stringifier::stringify(root, ostr);
            return; 
        }

        if (scheme == "Bearer"){
            std::string reason;
            try{
                if (!AuthHelper::ExtractPayload(info, user_id, user_login, reason)){
                    response.setStatus(Poco::Net::HTTPResponse::HTTPStatus::HTTP_UNAUTHORIZED);
                    response.setContentType("application/json");
                    Poco::JSON::Object::Ptr root = new Poco::JSON::Object();
                    root->set("type", "/errors/unauthorized");
                    root->set("title", "Internal exception");
                    root->set("status", "401");
                    root->set("detail", "JWT-token processing failed: " + reason);
                    root->set("instance", "/post");
                    std::ostream &ostr = response.send();
                    Poco::JSON::Stringifier::stringify(root, ostr);
                    return;                  
                }
            }
            catch(...){
                response.setStatus(Poco::Net::HTTPResponse::HTTPStatus::HTTP_UNAUTHORIZED);
                response.setContentType("application/json");
                Poco::JSON::Object::Ptr root = new Poco::JSON::Object();
                root->set("type", "/errors/unauthorized");
                root->set("title", "Internal exception");
                root->set("status", "401");
                root->set("detail", "JWT-token processing failed, only use token given from /user/auth GET request");
                root->set("instance", "/post");
                std::ostream &ostr = response.send();
                Poco::JSON::Stringifier::stringify(root, ostr);
                return;
            }
        }
        else
        {
            response.setStatus(Poco::Net::HTTPResponse::HTTPStatus::HTTP_UNAUTHORIZED);
            response.setContentType("application/json");
            Poco::JSON::Object::Ptr root = new Poco::JSON::Object();
            root->set("type", "/errors/unauthorized");
            root->set("title", "Internal exception");
            root->set("status", "401");
            root->set("detail", "authorization scheme must be <Bearer>");
            root->set("instance", "/post");
            std::ostream &ostr = response.send();
            Poco::JSON::Stringifier::stringify(root, ostr);
            return;
        }
        
        Poco::URI uri(request.getURI());

        if (uri.getPath() == "/post"
            && request.getMethod() == Poco::Net::HTTPRequest::HTTP_POST){
            Poco::Dynamic::Var jsonParseResult = PostHandler::_jsonParser.parse(request.stream());
            Poco::JSON::Object::Ptr json = jsonParseResult.extract<Poco::JSON::Object::Ptr>();

            json->set("author_id", user_id);
            std::string reason;
            if (CheckSaveData(json, reason)){
                database::Post post = database::Post::fromJSON(json);

                post.Save();

                response.setStatus(Poco::Net::HTTPResponse::HTTP_OK);
                response.setContentType("application/json");
                std::ostream &ostr = response.send();
                ostr << post.get_id();
            }
            else
            {
                response.setStatus(Poco::Net::HTTPResponse::HTTPStatus::HTTP_BAD_REQUEST);
                response.setContentType("application/json");
                Poco::JSON::Object::Ptr root = new Poco::JSON::Object();
                root->set("status", "400");
                root->set("detail", "post information is missing or incomplete: " + reason);
                root->set("instance", "/post");
                std::ostream &ostr = response.send();
                Poco::JSON::Stringifier::stringify(root, ostr);
                return;
            }

            return;
        }

        else if (uri.getPath() == "/post"
            && request.getMethod() == Poco::Net::HTTPRequest::HTTP_GET){
            Poco::URI::QueryParameters query = uri.getQueryParameters();

            bool isIdProvided = false;
            long id;
            for (std::pair<std::string, std::string> pair : query)
                if (pair.first == "id"){
                    id = std::stoi(pair.second);
                    isIdProvided = true;
                }
            
            if (id < 0){
                response.setStatus(Poco::Net::HTTPResponse::HTTPStatus::HTTP_BAD_REQUEST);
                response.setContentType("application/json");
                Poco::JSON::Object::Ptr root = new Poco::JSON::Object();
                root->set("status", "400");
                root->set("detail", "query parametr <id> is invalid");
                root->set("instance", "/post");
                std::ostream &ostr = response.send();
                Poco::JSON::Stringifier::stringify(root, ostr);
                return;
            }

            if (isIdProvided){
                std::optional<database::Post> post = database::Post::GetPostById(id);

                if (post){
                    response.setStatus(Poco::Net::HTTPResponse::HTTP_OK);
                    response.setContentType("application/json");
                    std::ostream &ostr = response.send();
                    Poco::JSON::Stringifier::stringify(post->toJSON(), ostr);
                }
                else
                {
                    response.setStatus(Poco::Net::HTTPResponse::HTTPStatus::HTTP_NOT_FOUND);
                    response.setContentType("application/json");
                    Poco::JSON::Object::Ptr root = new Poco::JSON::Object();
                    root->set("status", "404");
                    root->set("detail", "posts not found");
                    root->set("instance", "/post");
                    std::ostream &ostr = response.send();
                    Poco::JSON::Stringifier::stringify(root, ostr);
                    return;
                }

                return;
            }
            else
            {
                std::vector<database::Post> posts = database::Post::GetAllPosts();
                
                if (posts.size() == 0){
                    response.setStatus(Poco::Net::HTTPResponse::HTTPStatus::HTTP_NOT_FOUND);
                    response.setContentType("application/json");
                    Poco::JSON::Object::Ptr root = new Poco::JSON::Object();
                    root->set("status", "404");
                    root->set("detail", "post not found");
                    root->set("instance", "/post");
                    std::ostream &ostr = response.send();
                    Poco::JSON::Stringifier::stringify(root, ostr);
                    return;
                }

                Poco::JSON::Array jsonMessageArray;
                for (database::Post post : posts){
                    jsonMessageArray.add(post.toJSON());
                }

                response.setStatus(Poco::Net::HTTPResponse::HTTP_OK);
                response.setContentType("application/json");
                std::ostream &ostr = response.send();

                Poco::JSON::Stringifier::stringify(jsonMessageArray, ostr); 
            }

            return;
        }

        else if (uri.getPath() == "/post/user"
                 && request.getMethod() == Poco::Net::HTTPRequest::HTTP_GET){

            std::vector<database::Post> posts = database::Post::GetUserPosts(user_id);

            if (posts.size() == 0){
                response.setStatus(Poco::Net::HTTPResponse::HTTPStatus::HTTP_NOT_FOUND);
                response.setContentType("application/json");
                Poco::JSON::Object::Ptr root = new Poco::JSON::Object();
                root->set("status", "404");
                root->set("detail", "posts not found");
                root->set("instance", "/post/user");
                std::ostream &ostr = response.send();
                Poco::JSON::Stringifier::stringify(root, ostr);
                return;
            }

            Poco::JSON::Array jsonUserArray;
            for (database::Post post : posts){
                jsonUserArray.add(post.toJSON());
            }

            response.setStatus(Poco::Net::HTTPResponse::HTTP_OK);
            response.setContentType("application/json");
            std::ostream &ostr = response.send();

            Poco::JSON::Stringifier::stringify(jsonUserArray, ostr);
        }

        else if (uri.getPath() == "/post"
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
            
            Poco::Dynamic::Var json1 = PostHandler::_jsonParser.parse(request.stream());
            Poco::JSON::Object::Ptr json = json1.extract<Poco::JSON::Object::Ptr>();

            std::string reason;
            if (CheckUpdateData(isIdProvided, id, json, reason)){
                std::string text_content = json->get("text_content");
                database::Post::UpdatePost(id, text_content);

                response.setStatus(Poco::Net::HTTPResponse::HTTP_OK);
                response.setContentType("application/json");
                std::ostream &ostr = response.send();
                ostr << id;
            }
            else{
                response.setStatus(Poco::Net::HTTPResponse::HTTPStatus::HTTP_BAD_REQUEST);
                response.setContentType("application/json");
                Poco::JSON::Object::Ptr root = new Poco::JSON::Object();
                root->set("status", "400");
                root->set("detail", "update data is missing or incomplete: " + reason);
                root->set("instance", "/post");
                std::ostream &ostr = response.send();
                Poco::JSON::Stringifier::stringify(root, ostr);

                return;
            }

            return;
        }

        else if (uri.getPath() == "/post"
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

            if (id < 0){
                response.setStatus(Poco::Net::HTTPResponse::HTTPStatus::HTTP_BAD_REQUEST);
                response.setContentType("application/json");
                Poco::JSON::Object::Ptr root = new Poco::JSON::Object();
                root->set("status", "400");
                root->set("detail", "query parametr <id> is invalid");
                root->set("instance", "/post");
                std::ostream &ostr = response.send();
                Poco::JSON::Stringifier::stringify(root, ostr);
                return;
            }

            if (isIdProvided){
                database::Post::DeletePost(id);

                response.setStatus(Poco::Net::HTTPResponse::HTTP_OK);
                response.setContentType("application/json");
                std::ostream &ostr = response.send();
                ostr << id;
            }
            else{
                response.setStatus(Poco::Net::HTTPResponse::HTTPStatus::HTTP_BAD_REQUEST);
                response.setContentType("application/json");
                Poco::JSON::Object::Ptr root = new Poco::JSON::Object();
                root->set("status", "400");
                root->set("detail", "query parametr <id> is missing");
                root->set("instance", "/post");
                std::ostream &ostr = response.send();
                Poco::JSON::Stringifier::stringify(root, ostr);

                return;
            }

            return;
        }

        else{
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

bool PostHandler::CheckSaveData(const Poco::JSON::Object::Ptr &json, std::string &reason)
{
    reason = "";
    bool isOK = true;
    
    if (!(json->has("id")) || json->getValue<long>("id") <= 0){
        reason += "filed <id> in JWT-token payload data is invalid";
        isOK = false;
    }
    if (!(json->has("author_id"))){
        if (!reason.empty())
            reason += ", ";
        reason += "filed <id> in JWT-token payload is invalid";
        isOK = false;
    }
    if (!(json->has("text_content"))){
        if (!reason.empty())
            reason += ", ";
        reason += "filed <text_content> is missing in JSON body data";
        isOK = false;
    }
    
    return isOK;
}

bool PostHandler::CheckUpdateData(const bool &isIdProvided, const long &id, const Poco::JSON::Object::Ptr &json, std::string &reason)
{
    reason = "";
    bool isOK = true;
    
    if (!isIdProvided){
        reason += "query parametr <id> is missing";
        isOK = false;
    }
    if (id < 0){
        if (!reason.empty())
            reason += ", ";
        reason += "query parametr <id> is invalid";
        isOK = false;
    }
    if (!(json->has("text_content"))){
        if (!reason.empty())
            reason += ", ";
        reason += "filed <text_content> is missing in JSON body data";
        isOK = false;
    }
    
    return isOK;
}