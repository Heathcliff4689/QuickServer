#include <functional>
#include <iostream>
#include <algorithm>
#include <sys/epoll.h>
#include <sys/time.h>
#include <unistd.h>
#include <queue>
#include <vector>


class Timer
{
  public:
    Timer(unsigned long long expire, std::function<void(void)> fun, void *args)
        : expire_(expire), fun(fun)
    {
    }

    inline void active() { fun(); }

    inline unsigned long long getExpire() const{ return expire_; }

  private:
    std::function<void(void)> fun;
    void *args;

    unsigned long long expire_;
};


class TimerManager
{
public:
    TimerManager() {}

    Timer *addTimer(int timeout, std::function<void(void)> fun, void *args = NULL)
    {
        if(timeout <= 0)
            return NULL;

        unsigned long long now = getCurrentMillisecs();
        Timer* timer = new Timer( now+timeout, fun, args);

        queue_.push(timer);

        return timer;
    }

    void delTimer(Timer* timer)
    {
        std::priority_queue<Timer*,std::vector<Timer*>,cmp> newqueue;

        while( !queue_.empty() )
        {
            Timer* top = queue_.top();
            queue_.pop();
            if( top != timer )
                newqueue.push(top);
        }

        queue_ = newqueue;
    }

    unsigned long long getRecentTimeout()
    {
        unsigned long long timeout = -1;
        if( queue_.empty() )
            return timeout;

        unsigned long long now = getCurrentMillisecs();
        timeout = queue_.top()->getExpire() - now;
        if(timeout < 0)
            timeout = 0;

        return timeout;
    }

    void takeAllTimeout()
    {   
        unsigned long long now = getCurrentMillisecs();

        while ( !queue_.empty() )
        {
            Timer* timer = queue_.top();
            if( timer->getExpire() <= now )
            {
                queue_.pop();
                timer->active();
                delete timer;

                continue;
            }

            return;
        }
    }

    unsigned long long getCurrentMillisecs()
    {
        struct timespec ts;
        clock_gettime(CLOCK_MONOTONIC_COARSE, &ts);
        return ts.tv_sec * 1000 + ts.tv_nsec / (1000 * 1000);
    }

private:

    struct cmp
    {
        bool operator()(Timer*& lhs, Timer*& rhs) const { return lhs->getExpire() > rhs->getExpire(); }
    };

    std::priority_queue<Timer*,std::vector<Timer*>,cmp> queue_;
};