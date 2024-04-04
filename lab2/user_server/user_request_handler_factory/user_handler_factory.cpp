#include "user_handler_factory.h"
#include "../user_request_handler/user_handler.h"

Poco::Net::HTTPRequestHandler* UserHandlerFactory::createRequestHandler(const Poco::Net::HTTPServerRequest & request){
    return new MessageHandler;
}