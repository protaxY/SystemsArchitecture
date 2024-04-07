#include "message_handler_factory.h"
#include "../message_request_handler/message_handler.h"

Poco::Net::HTTPRequestHandler* MessageHandlerFactory::createRequestHandler(const Poco::Net::HTTPServerRequest & request){
    return new MessageHandler;
}