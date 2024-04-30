#ifndef USEHANDLER_H
#define USEHANDLER_H

#include <string>

#include <Poco/Net/HTTPRequestHandler.h>
#include <Poco/Net/HTTPServerRequest.h>
#include <Poco/Net/HTTPServerResponse.h>
#include <Poco/JSON/Parser.h>

class MessageHandler : public Poco::Net::HTTPRequestHandler{
    private:       
        Poco::JSON::Parser _jsonParser;

        static bool CheckSaveData(const Poco::JSON::Object::Ptr &json, std::string &reason);
        static bool CheckSearchByFirstLastName(const bool &isFirstNameProvided, const bool &isLastNameProvided, std::string &reason);
        static bool CheckUpdateData(const bool &isLoginProvided, const bool &isPasswordProvided, const Poco::JSON::Object::Ptr &json, std::string &reason);
        static bool CheckDeleteData(const bool &isLoginProvided, const bool &isPasswordProvided, std::string &reason);

    public:
        void handleRequest(Poco::Net::HTTPServerRequest &request, Poco::Net::HTTPServerResponse &response) override;
};

#endif // USEHANDLER_H