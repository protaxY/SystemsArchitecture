#ifndef POST_H
#define POST_H

#include <Poco/JSON/Object.h>
#include <Poco/DateTime.h>

#include <string>
#include <optional>

namespace database
{
    class Post{
        private:
            long _id;
            long _author_id;
            std::string _text_content;
            Poco::DateTime _send_time;

        public:
            static const std::string collectionName;
            static const std::string timeFormat;
            static const int timeZoneDifferential;
            
            static Post fromJSON(const Poco::JSON::Object::Ptr &json);
            Poco::JSON::Object::Ptr toJSON();

            const long           &get_id() const;
            const long           &get_author_id() const;
            const std::string    &get_text_content() const;
            const Poco::DateTime &get_send_time() const;

            long           &id();
            long           &author_id();
            std::string    &text_content();
            Poco::DateTime &send_time();

            void Save();
            static std::optional<Post> GetPostById(long &id);
            static std::vector<Post> GetAllPosts();
            static std::vector<Post> GetUserPosts(long &userId);
            static void UpdatePost(long &id, std::string &text_content);
            static void DeletePost(long &id);
    };
} // namespace database

#endif // POST_H