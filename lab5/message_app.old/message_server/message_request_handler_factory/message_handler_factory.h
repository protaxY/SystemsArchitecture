#ifndef HTTPMESSAGEREQUESTFACTORY_H
#define HTTPMESSAGEREQUESTFACTORY_H

#include <Poco/Net/HTTPRequestHandlerFactory.h>

class MessageHandlerFactory: public Poco::Net::HTTPRequestHandlerFactory
{
private:
    std::string _format;
    
public:
    Poco::Net::HTTPRequestHandler* createRequestHandler(const Poco::Net::HTTPServerRequest & request);
};

#endif // HTTPMESSAGEREQUESTFACTORY_H