#include "api_gateway_application.h"
#include "api_gateway_request_handler_factory/api_gateway_factory.h"

#include <Poco/Net/ServerSocket.h>
#include <Poco/Net/HTTPServer.h>
#include <Poco/Net/HTTPServerParams.h>
#include <Poco/DateTimeFormat.h>

#include <iostream>

int APIGatewayApplication::main([[maybe_unused]] const std::vector<std::string> &args){       
    Poco::Net::ServerSocket svs(Poco::Net::SocketAddress("0.0.0.0", 8080));
    Poco::Net::HTTPServer srv(new APIGatewayHandlerFactory, svs, new Poco::Net::HTTPServerParams);
    
    srv.start();
    waitForTerminationRequest();
    srv.stop();

    return Application::EXIT_OK;
}