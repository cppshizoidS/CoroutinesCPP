#include "thread_pool.h"

#include <iostream>
#include <thread>
#include <functional>
#include <stdexcept>
#include <mutex>

coros::base::ThreadPool::ThreadPool() : max_threads(std::thread::hardware_concurrency()) {
    is_shutdown = false;
}

coros::base::ThreadPool::ThreadPool(int max_threads) : max_threads(max_threads) {
    is_shutdown = false;    
}

void coros::base::ThreadPool::run_jobs() {
    while (true) {
        std::function<void()> job;
        {
            std::unique_lock<std::mutex> jobs_lock(jobs_mutex);
            jobs_condition.wait(jobs_lock, [&] { return !jobs.empty() || is_shutdown;});
            if (jobs.empty() && is_shutdown) {
                return;
            }
            job = jobs.front();
            jobs.pop();
        }
        try {
            job();
        } catch (std::runtime_error err) {
            std::cerr << err.what() << std::endl;
        }
    }
}

void coros::base::ThreadPool::run(std::function<void()> job) {
    {
        std::lock_guard<std::mutex> guard(jobs_mutex);
        if (is_shutdown) {
            throw std::runtime_error("ThreadPool run() error: ThreadPool already shutdown");
        }
        jobs.push(job);
        if (threads.size() < max_threads) {
            threads.push_back(std::thread(&ThreadPool::run_jobs, this));
        }
    }
    jobs_condition.notify_one();
}

void coros::base::ThreadPool::shutdown() {
    {
        std::lock_guard<std::mutex> guard(jobs_mutex);
        if (is_shutdown) {
            throw std::runtime_error("ThreadPool shutdown() error: ThreadPool already shutdown");
        }
        is_shutdown = true;
    }
    jobs_condition.notify_all();
    for (auto it = threads.begin(); it != threads.end(); ++it) {
        it->join();
    }
}
