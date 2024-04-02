#ifndef USER_H
#define USER_H

#include <Poco/JSON/Object.h>

#include <string>
#include <optional>

namespace database
{
    class User{
        private:
            long _id;
            std::string _first_name;
            std::string _last_name;
            std::string _email;
            std::string _title;
            std::string _login;
            std::string _password;

            void HashPassword();
            void RemovePassword();

        public:
            static User fromJSON(const Poco::JSON::Object::Ptr &json, const bool skipId = false);
            Poco::JSON::Object::Ptr toJSON();

            static bool IsNameValid(const std::string &name, std::string &reason);
            static bool IsEmailValid(const std::string &email, std::string &reason);

            long               get_id() const;
            const std::string &get_first_name() const;
            const std::string &get_last_name() const;
            const std::string &get_email() const;
            const std::string &get_title() const;
            const std::string &get_login() const;
            const std::string &get_password() const;

            long&        id();
            std::string &first_name();
            std::string &last_name();
            std::string &email();
            std::string &title();
            std::string &login();
            std::string &password();

            void Save();
            static std::optional<long> Update(std::string &login, std::string &oldHashedPassword, const Poco::JSON::Object::Ptr &json);
            static std::optional<User> SearchByLogin(std::string &login);
            static std::vector<User> SearchByFirstLastName(std::string firstName, std::string lastName);
            static std::optional<long> Delete(std::string &login, std::string &password);
            static std::vector<User> GetAllUsers();

            static std::string HashPassword(const std::string &password);
    };
} // namespace database

#endif // USER_H