#include "thread_pool.h"

#include <vector>
#include <queue>
#include <mutex>
#include <thread>
#include <condition_variable>
#include <functional>
#include <random>

using namespace std;

ThreadPool::ThreadPool (size_t threads){
        for (size_t i = 0; i < threads; ++i){
            workers.emplace_back([this] {
                thread_local mt19937 rng(random_device{}());
                while (true){
                    function<void(mt19937&)> task;
                    {
                        unique_lock lock(queue_mutex);
                        condition.wait(lock, [this]{return stop || !tasks.empty();});
                        if (stop && tasks.empty()) return;
                        task = move(tasks.front());
                        tasks.pop();
                        ++active_tasks;
                    }
                    task(rng);
                    {
                    std::unique_lock lock(queue_mutex);
                    --active_tasks;
                    if (tasks.empty() && active_tasks == 0)
                        done_condition.notify_all(); // wake wait_all()
                    }
                }
            });
        } 
    }

void ThreadPool::wait_all(){
    std::unique_lock lock(queue_mutex);
    done_condition.wait(lock, [this] {
        return tasks.empty() && active_tasks == 0;
    });
}

ThreadPool::~ThreadPool(){
    {
        unique_lock lock(queue_mutex);
        stop = true;
    }
    condition.notify_all();
    for (auto& w : workers) w.join();
}