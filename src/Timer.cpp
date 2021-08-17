#include "Timer.h"
#include <iostream>

Timer::Timer(unsigned long long expire, timerCall timer_call, int id) : _expire(expire),
                                                                _timer_call(timer_call),
                                                                id(id)
{
}
Timer::Timer() : _expire(0), _timer_call(), id(-1)
{
}
Timer::~Timer() {}

void Timer::active()
{
    _timer_call();
}

unsigned long long Timer::getExpire() const
{
    return _expire;
}

TimerManager::TimerManager() {}
TimerManager::~TimerManager() {}

std::shared_ptr<Timer> TimerManager::addTimer(int time_out, int key, timerCall call)
{
    if (time_out <= 0)
        return std::make_shared<Timer>();
    unsigned long long now = getCurrentMillisecs();
    auto timer = std::make_shared<Timer>(now + time_out, call, key);
    rbtree.insert(timer);
    return timer;
}

void TimerManager::delTimer(int key)
{
    for(auto i: rbtree)
    {
        if(i->id == key)
        {
            rbtree.erase(i);
            break;
        }
    }
}

void TimerManager::takeAllTimeout()
{
    unsigned long long now = getCurrentMillisecs();
    for(auto timer: rbtree)
    {
        std::cout<< timer->id<<": "<<timer->getExpire() - now<<"\n";
    }

    while(!rbtree.empty())
    {
        auto timer = *rbtree.begin();
        if( timer->getExpire() <= now)
        {
            rbtree.erase(rbtree.begin());
            timer->active();
        }
        else
        {
            break;
        }
    }
}

unsigned long long TimerManager::getRecentTimeout()
{
    unsigned long long timeout = -1;
    if(!rbtree.empty())
    {
        return timeout;
    }

    timeout = rbtree.begin()->get()->getExpire() - getCurrentMillisecs();
    if(timeout < 0)
    {
        timeout = 0;
    }

    return timeout;
}

unsigned long long TimerManager::getCurrentMillisecs()
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC_COARSE, &ts);
    return ts.tv_sec * 1000 + ts.tv_nsec / (1000 * 1000);
}
