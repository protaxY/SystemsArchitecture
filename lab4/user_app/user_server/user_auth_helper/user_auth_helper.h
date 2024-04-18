#ifndef USERAUTHHELPER_H
#define USERAUTHHELPER_H

#include <string>

class UserAuthHelper{
    public:
        static void DecodeIdentity(const std::string &identity, std::string &login, std::string &password);
        static std::string GenerateTocken(const long &id, const std::string &login);
};

#endif // USERAUTHHELPER_H