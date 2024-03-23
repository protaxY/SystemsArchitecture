#include "user_server_application.h"
#include "../database/database.h"
#include "user_request_handler_factory/user_handler_factory.h"

#include <Poco/Net/ServerSocket.h>
#include <Poco/Net/HTTPServer.h>
#include "Poco/Net/HTTPServerParams.h"
#include "Poco/DateTimeFormat.h"

#include <iostream>

int UserServerApplication::main([[maybe_unused]] const std::vector<std::string> &args){    
    
    std::cout << "here" << std::endl;
    
    database::Database::get().CreateTableIfNotExists();

    std::cout << "here2" << std::endl;
    
    Poco::Net::ServerSocket svs(Poco::Net::SocketAddress("0.0.0.0", 8080));
    Poco::Net::HTTPServer srv(new UserHandlerFactory, svs, new Poco::Net::HTTPServerParams);
    srv.start();
    waitForTerminationRequest();
    srv.stop();

    return Application::EXIT_OK;
}