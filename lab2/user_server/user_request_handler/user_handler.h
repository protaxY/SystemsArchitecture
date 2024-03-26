#ifndef USEHANDLER_H
#define USEHANDLER_H

#include <string>

#include <Poco/Net/HTTPRequestHandler.h>
#include <Poco/Net/HTTPServerRequest.h>
#include <Poco/Net/HTTPServerResponse.h>
#include <Poco/JSON/Parser.h>

class UserHandler : public Poco::Net::HTTPRequestHandler{
    private:       
        Poco::JSON::Parser _jsonParser;

    public:
        void handleRequest(Poco::Net::HTTPServerRequest &request, Poco::Net::HTTPServerResponse &response) override;
};

#endif // USEHANDLER_H