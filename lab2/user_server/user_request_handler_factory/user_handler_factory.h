#ifndef HTTPREQUESTFACTORY_H
#define HTTPREQUESTFACTORY_H

#include <Poco/Net/HTTPRequestHandlerFactory.h>

class UserHandlerFactory: public Poco::Net::HTTPRequestHandlerFactory
{
private:
    std::string _format;
    
public:
    Poco::Net::HTTPRequestHandler* createRequestHandler(const Poco::Net::HTTPServerRequest & request);
};

#endif