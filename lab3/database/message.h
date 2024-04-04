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
            Poco::DateTime send_date;

        public:
            static Message fromJSON(const Poco::JSON::Object::Ptr &json, const bool skipId = false);

            const long           &et_id() const;
            const long           &get_sender_id() const;
            const long           &get_receiver_id() const;
            const std::string    &get_text_content() const;
            const Poco::DateTime &get_send_date() const;

            long           &id();
            long           &sender_id();
            long           &receiver_id();
            std::string    &text_content();
            Poco::DateTime &send_date();

            void Save();
            static std::optional<Message> GetMessageById(long & id);
            static void UpdateMessage(long & id, long & userId, std::string & userHashedPassword);
            static void DeleteMessage(long & id);
            static std::vector<Message> GetUserMessages(long & userId, int & messagesNumber, std::optional<Poco::DateTime> & dateOffset);
    };
} // namespace database

#endif // MESSAGE_H