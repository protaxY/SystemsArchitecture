#include "api_gateway_handler.h"

#include <Poco/Net/HTTPServerRequest.h>
#include <Poco/URI.h>
#include <Poco/RegularExpression.h>
#include <Poco/Net/HTMLForm.h>
#include <Poco/StreamCopier.h>
#include <Poco/Net/HTTPClientSession.h>
#include <Poco/Net/SocketAddress.h>

#include <iostream>

#include "../../config/config.h"
#include "../../cache/cache.h"

void APIGatewayHandler::handleRequest(Poco::Net::HTTPServerRequest &request, Poco::Net::HTTPServerResponse &response)
{   
    Poco::URI uri(request.getURI());
    
    try
    {      
        bool hasCredentials = false;
        
        std::string scheme;
        std::string info;
        try{
            request.getCredentials(scheme, info);
            hasCredentials = true;
        }
        catch (...){}

        if (scheme != "Basic")
        {
            response.setStatus(Poco::Net::HTTPResponse::HTTPStatus::HTTP_UNAUTHORIZED);
            response.setChunkedTransferEncoding(true);
            response.setContentType("application/json");
            Poco::JSON::Object::Ptr root = new Poco::JSON::Object();
            root->set("type", "/errors/unauthorized");
            root->set("title", "Internal exception");
            root->set("status", "401");
            root->set("detail", "authorization scheme must be <Basic>");
            root->set("instance", "/api_gateway");
            std::ostream &ostr = response.send();
            Poco::JSON::Stringifier::stringify(root, ostr);
            return;
        }

        // проверить, есть ли в кэше
        if(request.getMethod() == Poco::Net::HTTPRequest::HTTP_GET) {
            std::optional<std::string> cachedResult = GetFromCache(request.getMethod(), request.getURI(), info);

            if (cachedResult){
                // std::cout << "# api gateway - from cache : " << cachedResult << std::endl;
                response.setStatus(Poco::Net::HTTPResponse::HTTP_OK);
                response.setChunkedTransferEncoding(true);
                response.setContentType("application/json");
                std::ostream &ostr = response.send();
                ostr << *cachedResult;
                ostr.flush();
                return;
            }
        }
        
        std::string authResult;
        std::string authScheme;
        if (Poco::URI(request.getURI()).getPath().length() >= 5
            && Poco::URI(request.getURI()).getPath().substr(0, 5) == "/user"
            && hasCredentials){
            authResult = info;
            authScheme = "Basic";
        }
        else if (hasCredentials && !APIGatewayHandler::AuthRequest(info, authResult)){
            response.setStatus(Poco::Net::HTTPResponse::HTTPStatus::HTTP_UNAUTHORIZED);
            response.setChunkedTransferEncoding(true);
            response.setContentType("application/json");
            std::ostream &ostr = response.send();
            ostr << authResult;
            return;
        }
        else
            authScheme = "Bearer";

        std::string &token = authResult;
        std::stringstream bodyStream;
        Poco::StreamCopier::copyStream(request.stream(), bodyStream);

        std::string result;
        Poco::Net::HTTPResponse::HTTPStatus redirectedStatus = RedirectRequest(request.getMethod(), Poco::URI(request.getURI()), bodyStream.str(), authScheme, token, result);
        if (redirectedStatus == Poco::Net::HTTPResponse::HTTPStatus::HTTP_OK
            && request.getMethod() == Poco::Net::HTTPRequest::HTTP_GET){
            PutToCache(request.getMethod(), request.getURI(), info, result);
        }
        
        response.setStatus(redirectedStatus);
        response.setChunkedTransferEncoding(true);
        response.setContentType("application/json");
        std::ostream &ostr = response.send();
        ostr << result;
        return;
    }
    catch (const Poco::JSON::JSONException e){
        response.setStatus(Poco::Net::HTTPResponse::HTTPStatus::HTTP_BAD_REQUEST);
        response.setChunkedTransferEncoding(true);
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
        response.setChunkedTransferEncoding(true);
        response.setContentType("application/json");
        Poco::JSON::Object::Ptr root = new Poco::JSON::Object();
        root->set("status", "500");
        root->set("detail", "unexpected error");
        root->set("instance", uri.getPath()); 
        std::ostream &ostr = response.send();
        Poco::JSON::Stringifier::stringify(root, ostr);        
    }
}

std::string APIGatewayHandler::AssembleKey(const std::string &method, const std::string &uri, const std::string &basicAuth){
    return method+":"+uri+":"+basicAuth;
}

void APIGatewayHandler::PutToCache(const std::string &method, const std::string &uri, const std::string &basicAuth, const std::string &value){
    const std::string key = AssembleKey(method, uri, basicAuth);
    database::Cache::get().put(key, value);
}

std::optional<std::string> APIGatewayHandler::GetFromCache(const std::string &method, const std::string &uri, const std::string &basicAuth){
    const std::string key = AssembleKey(method, uri, basicAuth);
    std::string value;
    const bool isOk = database::Cache::get().get(key, value);

    if (isOk)
        return value;
    else
        return {};
}

bool APIGatewayHandler::AuthRequest(const std::string &basicAuth, std::string &result){
    try{
        std::string authUri = "/user/auth";
        Poco::Net::HTTPClientSession session("user_service", std::stoi(Config::get().get_user_port()));

        Poco::Net::HTTPRequest request(Poco::Net::HTTPRequest::HTTP_GET, authUri, Poco::Net::HTTPMessage::HTTP_1_1);
        request.set("Authorization", "Basic " + basicAuth);
        request.setContentType("application/json");

        session.sendRequest(request);

        Poco::Net::HTTPResponse response;
        std::istream &rs = session.receiveResponse(response);
        std::stringstream ss;
        Poco::StreamCopier::copyStream(rs, ss);

        result = ss.str();

        if (response.getStatus() != Poco::Net::HTTPResponse::HTTPStatus::HTTP_OK)
            return false;
        else
            return true;
    }
    catch (Poco::Exception &ex)
    {
        std::cerr << ex.displayText() << std::endl;
        return false;
    }
    return false;
}

Poco::Net::HTTPResponse::HTTPStatus APIGatewayHandler::RedirectRequest(const std::string &method, const Poco::URI &uri, const std::string &body, const std::string &authScheme, const std::string &token, std::string &result){
    try{
        std::string redirectUri;
        std::string path = uri.getPath();
        
        std::string redirectHost;
        std::string redirectPort;
        if (path.length() >= 5
            && path.substr(0, 5) == "/user"){
            redirectHost = Config::get().get_user_host();
            redirectPort = Config::get().get_user_port();
        }
        else if (path.length() >= 8
                 && path.substr(0, 8) == "/message"){
            redirectHost = Config::get().get_message_host();
            redirectPort = Config::get().get_message_port();
        }
        else if (path.length() >= 5
                 && path.substr(0, 5) == "/post"){
            redirectHost = Config::get().get_post_host();
            redirectPort = Config::get().get_post_port();
        }
        Poco::Net::HTTPClientSession session(redirectHost, std::stoi(redirectPort));

        redirectUri += uri.getPathAndQuery();

        Poco::Net::HTTPRequest request(method, redirectUri, Poco::Net::HTTPMessage::HTTP_1_1);
        if (!token.empty())
            request.set("Authorization", authScheme + " " + token);
        request.setContentType("application/json");
        request.setContentLength(body.length());
        session.sendRequest(request) << body;
        session.sendRequest(request);

        Poco::Net::HTTPResponse response;
        std::istream &rs = session.receiveResponse(response);

        // Вывод ответа на экран
        std::stringstream ss;
        Poco::StreamCopier::copyStream(rs, ss);

        result = ss.str();

        return response.getStatus();
    }
    catch (Poco::Exception &ex)
    {
        std::cout << ex.displayText() << std::endl;
        return Poco::Net::HTTPResponse::HTTPStatus::HTTP_INTERNAL_SERVER_ERROR;
    }
    return Poco::Net::HTTPResponse::HTTPStatus::HTTP_INTERNAL_SERVER_ERROR;
}
