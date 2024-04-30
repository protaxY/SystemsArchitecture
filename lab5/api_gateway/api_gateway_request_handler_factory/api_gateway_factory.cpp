#include "api_gateway_factory.h"
#include "../api_gateway_request_handler/api_gateway_handler.h"

Poco::Net::HTTPRequestHandler* APIGatewayHandlerFactory::createRequestHandler(const Poco::Net::HTTPServerRequest & request){
    return new APIGatewayHandler;
}