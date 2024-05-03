#ifndef AUTHHELPER_H
#define AUTHHELPER_H

#include <string>

#include <Poco/SharedPtr.h>

class AuthHelper{
    private:
        static std::string getJWTKey();

    public:
        static void DecodeIdentity(const std::string &identity, std::string &login, std::string &password);
        static std::string GenerateToken(const long &id, const std::string &login);

        static bool ExtractPayload(const std::string &token, long &id, std::string &login, std::string &reason);
};

#endif // AUTHHELPER_H