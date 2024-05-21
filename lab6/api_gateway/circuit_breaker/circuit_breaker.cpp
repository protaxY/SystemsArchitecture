#include "circuit_breaker.h"

bool CircuitBreaker::Check(const std::string &serviceName)
{
    if (services.find(serviceName) == std::end(services))
        return true;
    
    ServiceState &serviceState = services[serviceName];

    switch (serviceState.breakerState)
    {
        case BreakerState::open:
            {
                auto currentTime = std::chrono::high_resolution_clock::now();
                double openedTimePeriod = std::chrono::duration<double>(currentTime - serviceState.openedTime).count();
                
                if (openedTimePeriod >= _openTimeLimit){
                    serviceState.breakerState = BreakerState::semi_open;
                    serviceState.failCount = 0;
                    serviceState.successCount = 0;
                    return true;
                }
                return false;
            }
            break;  
        case BreakerState::semi_open:
            return true;
            break;
        case BreakerState::close:
            return true;
            break;
    }
    return false;
}

void CircuitBreaker::Fail(const std::string &serviceName)
{
    if (services.find(serviceName) == std::end(services)){
        ServiceState newServiceState;
        newServiceState.serviceName = serviceName;
        newServiceState.failCount = 1;
        services[newServiceState.serviceName] = newServiceState;

        return;
    }
    
    ServiceState &serviceState = services[serviceName];

    switch (serviceState.breakerState)
    {
        case BreakerState::semi_open:
            serviceState.successCount = 0;
            serviceState.breakerState = BreakerState::open;
            serviceState.openedTime = std::chrono::high_resolution_clock::now();
            break;
        case BreakerState::close:
            serviceState.failCount += 1;
            if (serviceState.failCount > _failLimit){
                serviceState.successCount = 0;
                serviceState.breakerState = BreakerState::open;
                serviceState.openedTime = std::chrono::high_resolution_clock::now();
            }
            break;
    }
}

void CircuitBreaker::Succses(const std::string &serviceName)
{
    if (services.find(serviceName) == std::end(services))
        return;
    
    ServiceState &serviceState = services[serviceName];

    if (serviceState.breakerState == BreakerState::semi_open){
        ++serviceState.successCount;
        if (serviceState.successCount > _succsesLimit){
            serviceState.breakerState = BreakerState::close;
            serviceState.failCount = 0;
            serviceState.successCount = 0;
        }
    }
}
