#ifndef HTTPMESSAGEREQUESTFACTORY_H
#define HTTPMESSAGEREQUESTFACTORY_H

#include <Poco/Net/HTTPRequestHandlerFactory.h>

class MessageHandlerFactory: public Poco::Net::HTTPRequestHandlerFactory
{
    public:
        Poco::Net::HTTPRequestHandler* createRequestHandler(const Poco::Net::HTTPServerRequest & request);
};

#endif // HTTPMESSAGEREQUESTFACTORY_H