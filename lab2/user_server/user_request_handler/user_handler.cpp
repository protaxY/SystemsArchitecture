#include "user_handler.h"

#include <Poco/Net/HTTPServerRequest.h>
#include <Poco/URI.h>
#include <Poco/RegularExpression.h>

#include <iostream>


void UserHandler::handleRequest(Poco::Net::HTTPServerRequest &request, Poco::Net::HTTPServerResponse &response){
    try
    {
        Poco::URI uri(request.getURI());
        Poco::RegularExpression re("\\/user\\/\\d+$");

        // создать нового пользователя
        if (uri.getPath() == "/user"
            && request.getMethod() == Poco::Net::HTTPRequest::HTTP_POST){
            std::cout << "4" << std::endl;
        }
        // получить пользователя по ID
        else if (re.match(uri.getPath())
                 && request.getMethod() == Poco::Net::HTTPRequest::HTTP_GET){
            std::cout << "2" << std::endl;
        }
        // поиск по маске фамилии и имени
        else if (uri.getPath() == "/user/search"
                 && request.getMethod() == Poco::Net::HTTPRequest::HTTP_GET){
            Poco::URI::QueryParameters query = uri.getQueryParameters();
            std::cout << "3" << std::endl;
            std::cout << query[0].first << " " << query[0].second << std::endl;
            std::cout << query[1].first << " " << query[1].second << std::endl;
        }

        response.setStatus(Poco::Net::HTTPResponse::HTTP_OK);
    }
    catch (...)
    {

    }
}