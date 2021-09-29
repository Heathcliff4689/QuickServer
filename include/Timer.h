#pragma once

#include <time.h>
#include <functional>
#include <set>
#include <memory>

typedef std::function<void()> timerCall;

class Timer : std::enable_shared_from_this<Timer>
{
public:
    int id;

public:
    Timer(unsigned long long expire, timerCall timer_call, int id);
    Timer();
    ~Timer();
    void active();
    unsigned long long getExpire() const;

private:
    timerCall _timer_call;
    unsigned long long _expire;
};

class TimerManager
{
public:
    TimerManager();
    ~TimerManager();
    std::shared_ptr<Timer> addTimer(int time_out, int key, timerCall call);
    void delTimer(int key);
    void takeAllTimeout();
    unsigned long long getRecentTimeout();
    unsigned long long getCurrentMillisecs();

private:
    struct cmp
    {
        int operator()(const std::shared_ptr<Timer>& lhs, const std::shared_ptr<Timer>& rhs)
            const
        {
            return lhs->getExpire() < rhs->getExpire();
        };
    };
    // key -> fd,  timer->time task   
    std::set<std::shared_ptr<Timer>, cmp> rbtree;
};
