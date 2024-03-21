#include "user_server_application.h"
#include "database/database.h"

#include <Poco/Net/ServerSocket.h>
#include <Poco/Net/HTTPServer.h>
#include "Poco/Net/HTTPServerParams.h"
#include "Poco/DateTimeFormat.h"

int UserServerApplication::main([[maybe_unused]] const std::vector<std::string> &args){
        
    database::Database::get().CreateTableIfNotExists();
    Poco::Net::ServerSocket svs(Poco::Net::SocketAddress("0.0.0.0", 8080));
    Poco::Net::HTTPServer srv(new HTTPRequestFactory(Poco::DateTimeFormat::SORTABLE_FORMAT), svs, new Poco::Net::HTTPServerParams);
    srv.start();
    waitForTerminationRequest();
    srv.stop();

    return Application::EXIT_OK;
}