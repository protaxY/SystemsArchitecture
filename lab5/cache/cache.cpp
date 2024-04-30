#include "cache.h"
#include "../config/config.h"

#include <mutex>

#include <redis-cpp/stream.h>
#include <redis-cpp/execute.h>

namespace database
{
    std::mutex _mtx;
    
    Cache::Cache()
    {
        // _stream = rediscpp::make_stream(Config::get().get_cache_host(), Config::get().get_cache_port());
        _stream = rediscpp::make_stream("123", "123");
    }

    Cache Cache::get()
    {
        static Cache _instance;
        return _instance;
    }

    void Cache::put(const std::string &id, const std::string &val)
    {
        std::lock_guard<std::mutex> lck(_mtx);
        rediscpp::value response = rediscpp::execute(*_stream, "set",
                                                     id,
                                                     val, "ex", "60");
    }

    bool Cache::get(const std::string & id, std::string &val)
    {
        try{
            std::lock_guard<std::mutex> lck(_mtx);
            rediscpp::value response = rediscpp::execute(*_stream, "get", id);

            if (response.is_error_message())
                return false;
            if (response.empty())
                return false;

            val = response.as<std::string>();
            return true;
        }
        catch(...){
            return false;
        }
    }
    
} // namespace database

