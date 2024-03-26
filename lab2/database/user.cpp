#include "user.h"
#include "database.h"

#include <Poco/JSON/Parser.h>
#include <Poco/Dynamic/Var.h>
#include <Poco/Data/Session.h>
#include <Poco/Data/PostgreSQL/PostgreSQLException.h>
#include <Poco/Data/RecordSet.h>
#include <Poco/Crypto/DigestEngine.h>
#include <Poco/PBKDF2Engine.h>
#include <Poco/HMACEngine.h>
#include <Poco/SHA1Engine.h>

namespace database
{   
    User User::fromJSON(const Poco::JSON::Object::Ptr &json, const bool skipId){
        User user;

        if (skipId)
            user.id() = json->getValue<long>("id");
        user.first_name() = json->getValue<std::string>("first_name");
        user.last_name() = json->getValue<std::string>("last_name");
        user.email() = json->getValue<std::string>("email");
        user.title() = json->getValue<std::string>("title");
        user.login() = json->getValue<std::string>("login");
        user.password() = json->getValue<std::string>("password");

        return user;
    }

    Poco::JSON::Object::Ptr User::toJSON(){
        Poco::JSON::Object::Ptr root = new Poco::JSON::Object();

        root->set("id", _id);
        root->set("first_name", _first_name);
        root->set("last_name", _last_name);
        root->set("email", _email);
        root->set("title", _title);
        root->set("login", _login);
        root->set("password", _password);

        return root;
    }

    bool User::IsNameValid(const std::string &name, std::string &reason){
        if (name.length() < 3)
        {
            reason = "Name must be at leas 3 signs";
            return false;
        }

        if (name.find(' ') != std::string::npos)
        {
            reason = "Name can't contain spaces";
            return false;
        }

        if (name.find('\t') != std::string::npos)
        {
            reason = "Name can't contain spaces";
            return false;
        }

        return true;
    }

    bool User::IsEmailValid(const std::string &email, std::string &reason)
    {
        if (email.find('@') == std::string::npos)
        {
            reason = "Email must contain @";
            return false;
        }

        if (email.find(' ') != std::string::npos)
        {
            reason = "EMail can't contain spaces";
            return false;
        }

        if (email.find('\t') != std::string::npos)
        {
            reason = "EMail can't contain spaces";
            return false;
        }

        return true;
    }

    const std::string &User::get_login() const
    {
        return _login;
    }

    const std::string &User::get_password() const
    {
        return _password;
    }

    std::string &User::login()
    {
        return _login;
    }

    std::string &User::password()
    {
        return _password;
    }

    long User::get_id() const
    {
        return _id;
    }

    const std::string &User::get_first_name() const
    {
        return _first_name;
    }

    const std::string &User::get_last_name() const
    {
        return _last_name;
    }

    const std::string &User::get_email() const
    {
        return _email;
    }

    const std::string &User::get_title() const
    {
        return _title;
    }

    long &User::id()
    {
        return _id;
    }

    std::string &User::first_name()
    {
        return _first_name;
    }

    std::string &User::last_name()
    {
        return _last_name;
    }

    std::string &User::email()
    {
        return _email;
    }

    std::string &User::title()
    {
        return _title;
    }

    void User::Save(){
        try
        {
            HashPassword();
            
            Poco::Data::Session session = database::Database::get().create_session();
            Poco::Data::Statement insert_stmt(session);

            insert_stmt << "INSERT INTO users (first_name,last_name,email,title,login,password)"
                           "VALUES($1, $2, $3, $4, $5, $6)"
                           "RETURNING id;",
                Poco::Data::Keywords::use(_first_name),
                Poco::Data::Keywords::use(_last_name),
                Poco::Data::Keywords::use(_email),
                Poco::Data::Keywords::use(_title),
                Poco::Data::Keywords::use(_login),
                Poco::Data::Keywords::use(_password),
                Poco::Data::Keywords::into(_id),
                Poco::Data::Keywords::now;
        }
        catch (Poco::Data::PostgreSQL::ConnectionException &e)
        {
            std::cout << "connection:" << e.what() << std::endl;
            throw;
        }
        catch (Poco::Data::PostgreSQL::StatementException &e)
        {
            std::cout << "statement:" << e.what() << std::endl;
            throw;
        }
    }

    void User::RemovePassword()
    {
        password() = "******";
    }

    std::optional<long> User::Update(std::string &login, std::string &oldPassword, const Poco::JSON::Object::Ptr &json){
        try
        {
            std::string oldHashedPassword = HashPassword(oldPassword);
            
            Poco::Data::Session session = database::Database::get().create_session();
            Poco::Data::Statement select_stmt(session);
            User user;

            select_stmt << "SELECT id, first_name, last_name, email, title, login, password FROM users WHERE login=$1 AND password=$2",
                            Poco::Data::Keywords::into(user.id()),
                            Poco::Data::Keywords::into(user.last_name()),
                            Poco::Data::Keywords::into(user.first_name()),
                            Poco::Data::Keywords::into(user.email()),
                            Poco::Data::Keywords::into(user.title()),
                            Poco::Data::Keywords::into(user.login()),
                            Poco::Data::Keywords::into(user.password()),
                            Poco::Data::Keywords::use(login),
                            Poco::Data::Keywords::use(oldHashedPassword),
                            Poco::Data::Keywords::range(0, 1),
                            Poco::Data::Keywords::now;

            Poco::Data::RecordSet rs(select_stmt);
            if (rs.moveFirst()){
                if (json->has("first_name")){
                    user.first_name() = json->getValue<std::string>("first_name");
                }
                if (json->has("last_name")){
                    user.last_name() = json->getValue<std::string>("last_name");
                }
                if (json->has("email")){
                    user.email() = json->getValue<std::string>("email");
                }
                if (json->has("title")){
                    user.title() = json->getValue<std::string>("title");
                }
                if (json->has("password")){
                    user.password() = json->getValue<std::string>("password");
                    user.HashPassword();
                }

                Poco::Data::Statement update_stmt(session);
                update_stmt << "UPDATE users SET first_name=$1, last_name=$2, email=$3, title=$4, password=$5 WHERE login=$6",
                                Poco::Data::Keywords::use(user.first_name()),
                                Poco::Data::Keywords::use(user.last_name()),
                                Poco::Data::Keywords::use(user.email()),
                                Poco::Data::Keywords::use(user.title()),
                                Poco::Data::Keywords::use(user.password()),
                                Poco::Data::Keywords::use(user.login()),
                                Poco::Data::Keywords::now;

                return user.get_id();
            }
            return {};
        }
        catch (Poco::Data::PostgreSQL::ConnectionException &e)
        {
            std::cout << "connection:" << e.what() << std::endl;
            throw;
        }
        catch (Poco::Data::PostgreSQL::StatementException &e)
        {
            std::cout << "statement:" << e.what() << std::endl;
            throw;
        }
    }

    std::optional<User> User::SearchByLogin(std::string &login){
        try
        {
            Poco::Data::Session session = database::Database::get().create_session();
            Poco::Data::Statement select_stmt(session);

            User user;

            select_stmt << "SELECT id, first_name, last_name, email, title, login, password FROM users WHERE login=$1",
                            Poco::Data::Keywords::into(user.id()),
                            Poco::Data::Keywords::into(user.last_name()),
                            Poco::Data::Keywords::into(user.email()),
                            Poco::Data::Keywords::into(user.title()),
                            Poco::Data::Keywords::into(user.login()),
                            Poco::Data::Keywords::into(user.password()),
                            Poco::Data::Keywords::use(login),
                            Poco::Data::Keywords::range(0, 1),
                            Poco::Data::Keywords::now;

            Poco::Data::RecordSet rs(select_stmt);
            if (rs.moveFirst()){
                user.RemovePassword();
                return user;
            }
        }
        catch (Poco::Data::PostgreSQL::ConnectionException &e)
        {
            std::cout << "connection:" << e.what() << std::endl;
            throw;
        }
        catch (Poco::Data::PostgreSQL::StatementException &e)
        {
            std::cout << "statement:" << e.what() << std::endl;
            throw;
        }
        return {};
    }

    std::vector<User> User::SearchByFirstLastName(std::string firstName, std::string lastName){
        try
        {
            Poco::Data::Session session = database::Database::get().create_session();
            Poco::Data::Statement select_stmt(session);

            std::vector<User> users;
            User user;

            firstName += "%";
            lastName += "%";

            select_stmt << "SELECT id, first_name, last_name, email, title, login, password FROM users WHERE first_name LIKE $1 AND last_name LIKE $2",
                            Poco::Data::Keywords::into(user._id),
                            Poco::Data::Keywords::into(user._last_name),
                            Poco::Data::Keywords::into(user._email),
                            Poco::Data::Keywords::into(user._title),
                            Poco::Data::Keywords::into(user._login),
                            Poco::Data::Keywords::into(user._password),
                            Poco::Data::Keywords::use(firstName),
                            Poco::Data::Keywords::use(lastName),
                            Poco::Data::Keywords::range(0, 1);

            while (!select_stmt.done())
            {
                if (select_stmt.execute())
                    user.RemovePassword();
                    users.push_back(user);
            }
            return users;
        }
        catch (Poco::Data::PostgreSQL::ConnectionException &e)
        {
            std::cout << "connection:" << e.what() << std::endl;
            throw;
        }
        catch (Poco::Data::PostgreSQL::StatementException &e)
        {
            std::cout << "statement:" << e.what() << std::endl;
            throw;
        }
    }

    std::optional<long> User::Delete(std::string &login, std::string &password){
        try
        {
            std::string hashedPassword = HashPassword(password);
            
            Poco::Data::Session session = database::Database::get().create_session();
            Poco::Data::Statement select_stmt(session);

            long id;
            select_stmt << "SELECT id FROM users WHERE login=$1 AND password=$2",
                            Poco::Data::Keywords::into(id),
                            Poco::Data::Keywords::use(login),
                            Poco::Data::Keywords::use(hashedPassword),
                            Poco::Data::Keywords::range(0, 1),
                            Poco::Data::Keywords::now;

            Poco::Data::RecordSet rs(select_stmt);
            if (rs.moveFirst()){
                Poco::Data::Statement delete_stmt(session);
                delete_stmt << "DELETE FROM users WHERE login=$1",
                                Poco::Data::Keywords::use(login),
                                Poco::Data::Keywords::now;

                return id;
            }
            else
                return {};
        }
        catch (Poco::Data::PostgreSQL::ConnectionException &e)
        {
            std::cout << "connection:" << e.what() << std::endl;
            throw;
        }
        catch (Poco::Data::PostgreSQL::StatementException &e)
        {
            std::cout << "statement:" << e.what() << std::endl;
            throw;
        }
    }

    std::string User::HashPassword(const std::string &password)
    {
        Poco::PBKDF2Engine<Poco::HMACEngine<Poco::SHA1Engine>> pbkdf2("some_salt", 4096, 128);
        
        pbkdf2.update(password);
        std::string result = Poco::DigestEngine::digestToHex(pbkdf2.digest());
        return result;
    }

    void User::HashPassword()
    {
        password() = HashPassword(password());
    }

} // namespace database
