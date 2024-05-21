#ifndef API_GATEWAY_HANDLER_FACTORY_H
#define API_GATEWAY_HANDLER_FACTORY_H

#include <Poco/Net/HTTPRequestHandlerFactory.h>

class APIGatewayHandlerFactory: public Poco::Net::HTTPRequestHandlerFactory
{
    public:
        Poco::Net::HTTPRequestHandler* createRequestHandler(const Poco::Net::HTTPServerRequest & request);
};

#endif // API_GATEWAY_HANDLER_FACTORY_H