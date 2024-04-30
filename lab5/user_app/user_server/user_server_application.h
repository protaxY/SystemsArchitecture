#ifndef USERSERVERAPP_H
#define USERSERVERAPP_H

#include <string>
#include <Poco/Util/ServerApplication.h>

class UserServerApplication : public Poco::Util::ServerApplication{
    public:
        int main(const std::vector<std::string> &args) override;
};

#endif // USERSERVERAPP_H