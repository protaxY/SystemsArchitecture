#ifndef CONFIG_H
#define CONFIG_H

#include <string>

class  Config{
    private:
        Config();

        std::string _mongo;
        std::string _mongo_port;
        std::string _mongo_database;

    public:
        static Config& get();

        std::string& mongo();
        std::string& mongo_port();
        std::string& mongo_database();

        const std::string& get_mongo() const;
        const std::string& get_mongo_port() const;
        const std::string& get_mongo_database() const;
};

#endif