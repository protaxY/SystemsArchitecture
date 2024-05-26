#ifndef CIRCUIT_BREAKER
#define CIRCUIT_BREAKER

#include <string>
#include <map>
#include <chrono>

enum class BreakerState
{
    close = 0,
    open = 1 ,
    semi_open = 2
};

struct ServiceState
{
    std::string serviceName;
    BreakerState breakerState{BreakerState::close};
    int failCount{0};
    int successCount{0};
    std::chrono::_V2::system_clock::time_point openedTime;
    std::chrono::_V2::system_clock::time_point lastFailureTime;
};

class CircuitBreaker{
    private:
        const double _openTimeLimit = 20;
        const double _failResetTimeLimit = 20; 
        const int _failLimit = 2;
        const int _succsesLimit = 2;
    
    public:
        std::map<std::string, ServiceState> services;
        bool Check(const std::string &serviceName);
        void Fail(const std::string &serviceName);
        void Succses(const std::string &serviceName);
};

#endif // CIRCUIT_BREAKER