#include "user_handler_factory.h"

#include <Poco/Net/HTTPServerRequest.h>
#include <Poco/URI.h>
#include <Poco/RegularExpression.h>

UserHandlerFactory::UserHandlerFactory(){}

Poco::Net::HTTPRequestHandler* UserHandlerFactory::createRequestHandler(const Poco::Net::HTTPServerRequest & request){
     try
     {
        Poco::URI uri(request.getURI());
        Poco::RegularExpression re("\/user\/\d+$");

        // создать нового пользователя
        if (uri.getPath() == "/user"
            && request.getMethod() == Poco::Net::HTTPRequest::HTTP_POST){

        }
        // получить пользователя по ID
        else if (re.match(uri.getPath())
                 && request.getMethod() == Poco::Net::HTTPRequest::HTTP_GET){
            
        }
        // поиск по маске фамилии и имени
        else if (uri.getPath() == "/user/search"
                 && request.getMethod() == Poco::Net::HTTPRequest::HTTP_GET){
            Poco::URI::QueryParameters query = uri.getQueryParameters();

        } 
     }
     catch (...)
     {

     }
}