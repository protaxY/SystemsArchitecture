#ifndef MESSAGE_H
#define MESSAGE_H

#include <Poco/JSON/Object.h>
#include <Poco/DateTime.h>

#include <string>
#include <optional>

namespace database
{
    class Message{
        private:

            long _id;
            long _sender_id;
            long _receiver_id;
            std::string _text_content;
            Poco::DateTime _send_time;

        public:
            static const std::string collectionName;
            static const std::string timeFormat;
            static const int timeZoneDifferential;
            
            static Message fromJSON(const Poco::JSON::Object::Ptr &json);
            Poco::JSON::Object::Ptr toJSON();

            const long           &get_id() const;
            const long           &get_sender_id() const;
            const long           &get_receiver_id() const;
            const std::string    &get_text_content() const;
            const Poco::DateTime &get_send_time() const;

            long           &id();
            long           &sender_id();
            long           &receiver_id();
            std::string    &text_content();
            Poco::DateTime &send_time();

            void Save();
            static std::optional<Message> GetMessageById(long &id);
            static std::vector<Message> GetAllMessages();
            static std::vector<Message> GetUserMessages(long &userId);
            static std::vector<Message> GetDialogMessages(long &firstUserId, long &secondUserId);
            static void UpdateMessage(long &id, std::string &text_content);
            static void DeleteMessage(long &id);
    };
} // namespace database

#endif // MESSAGE_H