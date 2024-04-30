#include "post_server_application.h"
#include "../../mongo_database/mongo_database.h"
#include "post_request_handler_factory/post_handler_factory.h"

#include <Poco/Net/ServerSocket.h>
#include <Poco/Net/HTTPServer.h>
#include <Poco/Net/HTTPServerParams.h>
#include <Poco/DateTimeFormat.h>

#include <iostream>

int PostServerApplication::main([[maybe_unused]] const std::vector<std::string> &args){       
    Poco::Net::ServerSocket svs(Poco::Net::SocketAddress("0.0.0.0", 8080));
    Poco::Net::HTTPServer srv(new PostHandlerFactory, svs, new Poco::Net::HTTPServerParams);
    
    srv.start();
    waitForTerminationRequest();
    srv.stop();

    return Application::EXIT_OK;
}