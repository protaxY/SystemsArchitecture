#ifndef POSTSERVERAPP_H
#define POSTSERVERAPP_H

#include <string>
#include <Poco/Util/ServerApplication.h>

class PostServerApplication : public Poco::Util::ServerApplication{
    public:
        int main(const std::vector<std::string> &args) override;
};

#endif // POSTSERVERAPP_H