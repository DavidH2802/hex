#pragma once

#include <vector>
#include <queue>
#include <mutex>
#include <thread>
#include <condition_variable>
#include <functional>
#include <random>

using namespace std;

struct ThreadPool {
    private:
        vector<thread> workers;
        queue<function<void(mt19937&)>> tasks;
        mutex queue_mutex;
        condition_variable condition;
        condition_variable done_condition; // for wait_all()
        int active_tasks = 0;
        bool stop = false;
    public:
        ThreadPool (size_t threads);

        template<class F>
        void enqueue(F&& f);

        void wait_all();

        ~ThreadPool();
};

template<class F>
void ThreadPool::enqueue(F&& f){
    {
        unique_lock lock(queue_mutex);
        tasks.emplace([fn = forward<F>(f)](mt19937& rng) mutable {fn(rng);});
    }
    condition.notify_one();
}