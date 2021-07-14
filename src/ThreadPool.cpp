#include "ThreadPool.h"

ThreadPool::ThreadPool(int n) : stop(false),
                                    threadnum(0),
                                    task_queue(),
                                    worker_list(),
                                    m(),
                                    cv()
{
    addWorkerThread(n);
}

ThreadPool::~ThreadPool()
{
    stopAll();   
}

void ThreadPool::addWorkerThread(int num)
{
    if (num > 0)
    {
        threadnum += num;
        for (int i = 0; i < num; ++i)
        {
            this->worker_list.push_back(
                std::thread(&ThreadPool::workerThread, this));
        }
    }
}

void ThreadPool::workerThread()
{
    while (!stop)
    {
        Callback callback;
        {
            std::unique_lock<std::mutex> lk(m);
            cv.wait(lk, [this]
                    {
                        if (!this->task_queue.empty() | stop)
                        {
                            return true;
                        }
                        return false;
                    });
            if (!this->task_queue.empty())
            {
                callback = task_queue.front();
                task_queue.pop();
            }
        }

        if (callback)
        {
            try
            {
                callback();
            }
            catch (std::bad_alloc &ba)
            {
                std::cerr << "bad_alloc caught in ThreadPool::ThreadFunc task: " << ba.what() << '\n';
                while (1)
                    ;
            }
        }
    }
}

void ThreadPool::addTask(Callback callback)
{
    {
        std::unique_lock<std::mutex> lk(m);
        task_queue.push(callback);
    }
    cv.notify_one();
}

void ThreadPool::stopAll()
{
    stop = true;
    // wake up all threads racing without blocking. 
    cv.notify_all(); 

    for (int i = 0; i < worker_list.size(); ++i)
    {
        worker_list[i].join();
    }
}
