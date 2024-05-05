#ifndef CONFIG_H
#define CONFIG_H

#include <string>

class  Config{
    private:
        Config();

        std::string _host;
        std::string _port;
        std::string _login;
        std::string _password;
        std::string _database;

        std::string _mongo;
        std::string _mongo_port;
        std::string _mongo_database;

        std::string _cache_host;
        std::string _cache_port;

        std::string _user_host;
        std::string _user_port;

        std::string _message_host;
        std::string _message_port;

        std::string _post_host;
        std::string _post_port;

    public:
        static Config& get();

        std::string& port();
        std::string& host();
        std::string& login();
        std::string& password();
        std::string& database();

        const std::string& get_port() const ;
        const std::string& get_host() const ;
        const std::string& get_login() const ;
        const std::string& get_password() const ;
        const std::string& get_database() const ;

        std::string& mongo();
        std::string& mongo_port();
        std::string& mongo_database();

        const std::string& get_mongo() const;
        const std::string& get_mongo_port() const;
        const std::string& get_mongo_database() const;

        const std::string& get_cache_host() const;
        const std::string& get_cache_port() const;

        const std::string& get_user_host() const;
        const std::string& get_user_port() const;

        const std::string& get_message_host() const;
        const std::string& get_message_port() const;

        const std::string& get_post_host() const;
        const std::string& get_post_port() const;
};

#endif