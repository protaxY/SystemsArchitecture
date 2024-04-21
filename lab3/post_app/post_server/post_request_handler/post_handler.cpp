#include "post_handler.h"
#include "../../../mongo_database/post.h"
#include "../../../mongo_database/mongo_database.h"

#include <Poco/Net/HTTPServerRequest.h>
#include <Poco/URI.h>
#include <Poco/RegularExpression.h>
#include <Poco/Net/HTMLForm.h>

#include <iostream>



void PoistHandler::handleRequest(Poco::Net::HTTPServerRequest &request, Poco::Net::HTTPServerResponse &response)
{   
    try
    {      
        Poco::URI uri(request.getURI());

        if (uri.getPath() == "/post"
            && request.getMethod() == Poco::Net::HTTPRequest::HTTP_POST){
            Poco::Dynamic::Var jsonParseResult = PoistHandler::_jsonParser.parse(request.stream());
            Poco::JSON::Object::Ptr json = jsonParseResult.extract<Poco::JSON::Object::Ptr>();

            std::string reason;
            if (CheckSaveData(json, reason)){
                database::Post post = database::Post::fromJSON(json);

                post.Save();

                response.setStatus(Poco::Net::HTTPResponse::HTTP_OK);
                response.setChunkedTransferEncoding(true);
                response.setContentType("application/json");
                std::ostream &ostr = response.send();
                ostr << post.get_id();
            }
            else
            {
                response.setStatus(Poco::Net::HTTPResponse::HTTPStatus::HTTP_BAD_REQUEST);
                response.setChunkedTransferEncoding(true);
                response.setContentType("application/json");
                Poco::JSON::Object::Ptr root = new Poco::JSON::Object();
                root->set("status", "400");
                root->set("detail", "Post information is missing or incomplete: " + reason);
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
                root->set("instance", "/post");
                std::ostream &ostr = response.send();
                Poco::JSON::Stringifier::stringify(root, ostr);
                return;
            }
            
            if (isIdProviuded){
                std::optional<database::Post> post = database::Post::GetPostById(id);

                if (post){
                    response.setStatus(Poco::Net::HTTPResponse::HTTP_OK);
                    response.setChunkedTransferEncoding(true);
                    response.setContentType("application/json");
                    std::ostream &ostr = response.send();
                    Poco::JSON::Stringifier::stringify(post->toJSON(), ostr);
                }
                else
                {
                    response.setStatus(Poco::Net::HTTPResponse::HTTPStatus::HTTP_NOT_FOUND);
                    response.setChunkedTransferEncoding(true);
                    response.setContentType("application/json");
                    Poco::JSON::Object::Ptr root = new Poco::JSON::Object();
                    root->set("status", "404");
                    root->set("detail", "Posts not found");
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
                    response.setChunkedTransferEncoding(true);
                    response.setContentType("application/json");
                    Poco::JSON::Object::Ptr root = new Poco::JSON::Object();
                    root->set("status", "404");
                    root->set("detail", "Post not found");
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
                response.setChunkedTransferEncoding(true);
                response.setContentType("application/json");
                std::ostream &ostr = response.send();

                Poco::JSON::Stringifier::stringify(jsonMessageArray, ostr); 
            }

            return;
        }

        else if (uri.getPath() == "/post/user"
                 && request.getMethod() == Poco::Net::HTTPRequest::HTTP_GET){
            Poco::URI::QueryParameters query = uri.getQueryParameters();

            bool isUserIdProviuded = false;
            long userId;
            for (std::pair<std::string, std::string> pair : query)
                if (pair.first == "id"){
                    userId = std::stoi(pair.second);
                    isUserIdProviuded = true;
                }

            if (userId <= 0){
                response.setStatus(Poco::Net::HTTPResponse::HTTPStatus::HTTP_BAD_REQUEST);
                response.setChunkedTransferEncoding(true);
                response.setContentType("application/json");
                Poco::JSON::Object::Ptr root = new Poco::JSON::Object();
                root->set("status", "400");
                root->set("detail", "query parametr <id> is invalid");
                root->set("instance", "/post/user");
                std::ostream &ostr = response.send();
                Poco::JSON::Stringifier::stringify(root, ostr);
                return;
            }

            if (isUserIdProviuded){
                std::vector<database::Post> posts = database::Post::GetUserPosts(userId);

                if (posts.size() == 0){
                    response.setStatus(Poco::Net::HTTPResponse::HTTPStatus::HTTP_NOT_FOUND);
                    response.setChunkedTransferEncoding(true);
                    response.setContentType("application/json");
                    Poco::JSON::Object::Ptr root = new Poco::JSON::Object();
                    root->set("status", "404");
                    root->set("detail", "Posts not found");
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
                root->set("instance", "/post/user");
                std::ostream &ostr = response.send();
                Poco::JSON::Stringifier::stringify(root, ostr);
                return;
            }
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
            
            Poco::Dynamic::Var json1 = PoistHandler::_jsonParser.parse(request.stream());
            Poco::JSON::Object::Ptr json = json1.extract<Poco::JSON::Object::Ptr>();

            std::string reason;
            if (!CheckUpdateData(isIdProvided, id, json, reason)){
                std::string text_content = json->get("text_content");
                database::Post::UpdatePost(id, text_content);

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
                root->set("detail", "Update information is missing or incomplete: " + reason);
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
                response.setChunkedTransferEncoding(true);
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
                root->set("instance", "/post");
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
        root->set("detail", "invalid JSON format");
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

bool PoistHandler::CheckSaveData(const Poco::JSON::Object::Ptr &json, std::string &reason)
{
    reason = "";
    bool isOK = true;
    
    if (!(json->has("id"))){
        reason += R"(filed <id> is missing in JSON body data)";
        isOK = false;
    }
    if (!(json->has("author_id"))){
        if (!reason.empty())
            reason += ", ";
        reason += R"(filed <author_id> is missing in JSON body data)";
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

bool PoistHandler::CheckUpdateData(const bool &isIdProvided, const long &id, const Poco::JSON::Object::Ptr &json, std::string &reason)
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
