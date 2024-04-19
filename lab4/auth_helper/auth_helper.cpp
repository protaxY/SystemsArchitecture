#include "auth_helper.h"

#include <string>
#include <sstream>
#include <Poco/Base64Decoder.h>
#include <Poco/JWT/Token.h>
#include <Poco/JWT/Signer.h>
#include <Poco/Crypto/RSAKey.h>

void AuthHelper::DecodeIdentity(const std::string &identity, std::string &login, std::string &password)
{
    std::istringstream istr(identity);
    std::ostringstream ostr;
    Poco::Base64Decoder b64in(istr);
    copy(std::istreambuf_iterator<char>(b64in),
         std::istreambuf_iterator<char>(),
         std::ostreambuf_iterator<char>(ostr));
    std::string decoded = ostr.str();

    size_t pos = decoded.find(':');
    login = decoded.substr(0, pos);
    password = decoded.substr(pos + 1);
}

std::string AuthHelper::GenerateToken(const long &id, const std::string &login)
{
    Poco::JWT::Token token;
    token.setType("JWT");
    token.setSubject("login");
    token.payload().set("login", login);
    token.payload().set("id", id);
    token.setIssuedAt(Poco::Timestamp());

    Poco::SharedPtr<Poco::Crypto::RSAKey> RSAKeyPtr(new Poco::Crypto::RSAKey("", "private", "pass"));
    // Poco::SharedPtr<Poco::Crypto::RSAKey> RSAKeyPtr(new Poco::Crypto::RSAKey("public", "private", "pass"));

    Poco::JWT::Signer signer(RSAKeyPtr);
    signer.addAlgorithm(Poco::JWT::Signer::ALGO_RS256);
    return signer.sign(token, Poco::JWT::Signer::ALGO_RS256);
}

bool AuthHelper::ExtractPayload(const std::string &token, long &id, std::string &login)
{
    Poco::SharedPtr<Poco::Crypto::RSAKey> RSAKeyPtr(new Poco::Crypto::RSAKey("public"));
    Poco::JWT::Signer signer(RSAKeyPtr);
    signer.addAlgorithm(Poco::JWT::Signer::ALGO_RS256);

    try {
        signer.verify(token);
        
        Poco::JWT::Token jwtTToken = signer.verify(token);

        if (jwtTToken.payload().has("login") && jwtTToken.payload().has("id")) {
            id = jwtTToken.payload().getValue<long>("id");
            login = jwtTToken.payload().getValue<std::string>("login");
            return true;
        }
        else {
            return false;
        }
    }
    catch (...) {
        return false;
    }
    return true;
}