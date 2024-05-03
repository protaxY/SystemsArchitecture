#ifndef API_GATEWAY_APP_H
#define API_GATEWAY_APP_H

#include <string>
#include <Poco/Util/ServerApplication.h>

class APIGatewayApplication : public Poco::Util::ServerApplication{
    public:
        int main(const std::vector<std::string> &args) override;
};

#endif // API_GATEWAY_APP_H