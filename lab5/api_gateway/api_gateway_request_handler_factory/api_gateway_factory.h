#ifndef HTTPPOSTREQUESTFACTORY_H
#define HTTPPOSTREQUESTFACTORY_H

#include <Poco/Net/HTTPRequestHandlerFactory.h>

class APIGatewayHandlerFactory: public Poco::Net::HTTPRequestHandlerFactory
{
private:
    std::string _format;
    
public:
    Poco::Net::HTTPRequestHandler* createRequestHandler(const Poco::Net::HTTPServerRequest & request);
};

#endif // HTTPPOSTREQUESTFACTORY_H