// #include "message_server/message_server_application.h"

#include <Poco/MongoDB/Database.h>
#include <Poco/MongoDB/QueryRequest.h>
#include <Poco/SharedPtr.h>
#include <Poco/MongoDB/Connection.h>

#include <Poco/MongoDB/RequestMessage.h>
#include <Poco/MongoDB/MessageHeader.h>

#include "message_server/message_server_application.h"

int main(int argc, char*argv[]){

    // Poco::MongoDB::Database db("arch");

    // Poco::MongoDB::Connection connection;

    // std::string host = "mongodb";
    // int port = 27017;

    // connection.connect(host.c_str(), port);

    // Poco::SharedPtr<Poco::MongoDB::QueryRequest> createRequest = db.createCommand();

    // createRequest->selector().add("create", "messages3");

    // connection.sendRequest(*createRequest);

    // // Poco::MongoDB::QueryRequest::send()

    MessageServerApplication app;
    return app.run(argc, argv);

    return 0;
}