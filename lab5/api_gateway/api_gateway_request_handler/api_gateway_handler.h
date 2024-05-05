#ifndef API_GATEWAY_HANDLER_H
#define API_GATEWAY_HANDLER_H

#include <string>
#include <optional>

#include <Poco/Net/HTTPRequestHandler.h>
#include <Poco/Net/HTTPServerRequest.h>
#include <Poco/Net/HTTPServerResponse.h>
#include <Poco/JSON/Parser.h>
#include <Poco/URI.h>

class APIGatewayHandler : public Poco::Net::HTTPRequestHandler{
    private:       
        Poco::JSON::Parser _jsonParser;
        static bool CheckSaveData(const Poco::JSON::Object::Ptr &json, std::string &reason);
        static bool CheckUpdateData(const bool &isIdProvided, const long &id, const Poco::JSON::Object::Ptr &json, std::string &reason);

        static std::string AssembleKey(const std::string &method, const std::string &uri, const std::string &basicAuth);
        static void PutToCache(const std::string &method, const std::string &uri, const std::string &basicAuth, const std::string &value);
        static std::optional<std::string> GetFromCache(const std::string &method, const std::string &uri, const std::string &basicAuth);

        static bool AuthRequest(const std::string &basicAuth, std::string &result);
        static Poco::Net::HTTPResponse::HTTPStatus RedirectRequest(const std::string &method, const Poco::URI &uri, const std::string &body, const std::string &authScheme, const std::string &token, std::string &result);

    public:
        void handleRequest(Poco::Net::HTTPServerRequest &request, Poco::Net::HTTPServerResponse &response) override;
};

#endif // API_GATEWAY_HANDLER_H