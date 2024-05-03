#ifndef HTTPPOSTREQUESTFACTORY_H
#define HTTPPOSTREQUESTFACTORY_H

#include <Poco/Net/HTTPRequestHandlerFactory.h>

class PostHandlerFactory: public Poco::Net::HTTPRequestHandlerFactory
{
    public:
        Poco::Net::HTTPRequestHandler* createRequestHandler(const Poco::Net::HTTPServerRequest & request);
};

#endif // HTTPPOSTREQUESTFACTORY_H