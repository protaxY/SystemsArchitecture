#ifndef USER_H
#define USER_H

#include <string>

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

        public:
            static User fromJSON(const std::string & str);

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

    };
} // namespace database

#endif // USER_H