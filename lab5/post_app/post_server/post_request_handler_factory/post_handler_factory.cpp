#include "post_handler_factory.h"
#include "../post_request_handler/post_handler.h"

Poco::Net::HTTPRequestHandler* PostHandlerFactory::createRequestHandler(const Poco::Net::HTTPServerRequest & request){
    return new PostHandler;
}