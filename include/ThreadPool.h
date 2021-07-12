#pragma once

/*
    TreadPool initializing is creating threads.
    author: zhangjie
*/

#include <iostream>
#include <functional>
#include <queue>
#include <vector>
#include <thread>
#include <mutex>
#include <condition_variable>

class ThreadPool
{
public:
    typedef std::function<void()> Callback;

    
    ThreadPool(int n = 4);
    ~ThreadPool(){};

    void addWorkerThread(int num);

    void workerThread();

    void addTask(Callback callback);

    void stopAll();

private:
    bool stop;
    int threadnum;

    std::mutex m;
    std::condition_variable cv;

    std::queue<Callback> task_queue;
    std::vector<std::thread> worker_list;
};