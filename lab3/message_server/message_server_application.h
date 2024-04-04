#ifndef USERSERVERAPP_H
#define USERSERVERAPP_H

#include <string>
#include <Poco/Util/ServerApplication.h>

class MessageServerApplication : public Poco::Util::ServerApplication{
    public:
        int main(const std::vector<std::string> &args) override;
};

#endif // MESSAGESERVERAPP_H