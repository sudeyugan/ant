#pragma once

#include <condition_variable>
#include <functional>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

namespace ant::ai_bridge {

class AsyncTaskQueue final {
public:
    using Task = std::function<void()>;

    AsyncTaskQueue() = default;
    ~AsyncTaskQueue();

    AsyncTaskQueue(const AsyncTaskQueue&) = delete;
    AsyncTaskQueue& operator=(const AsyncTaskQueue&) = delete;
    AsyncTaskQueue(AsyncTaskQueue&&) = delete;
    AsyncTaskQueue& operator=(AsyncTaskQueue&&) = delete;

    void Start(std::size_t worker_count = 1);
    void Stop();
    void Enqueue(Task task);

private:
    void WorkerLoop();

    std::mutex mutex_{};
    std::condition_variable cv_{};
    std::queue<Task> tasks_{};
    std::vector<std::thread> workers_{};
    bool running_{false};
};

} // namespace ant::ai_bridge

namespace ant::ai_bridge {

inline AsyncTaskQueue::~AsyncTaskQueue() {
    Stop();
}

inline void AsyncTaskQueue::Start(std::size_t worker_count) {
    if (running_) {
        return;
    }
    running_ = true;
    workers_.reserve(worker_count);
    for (std::size_t i = 0; i < worker_count; ++i) {
        workers_.emplace_back([this] { WorkerLoop(); });
    }
}

inline void AsyncTaskQueue::Stop() {
    {
        std::lock_guard<std::mutex> lock(mutex_);
        running_ = false;
    }
    cv_.notify_all();
    for (auto& worker : workers_) {
        if (worker.joinable()) {
            worker.join();
        }
    }
    workers_.clear();
}

inline void AsyncTaskQueue::Enqueue(Task task) {
    {
        std::lock_guard<std::mutex> lock(mutex_);
        tasks_.push(std::move(task));
    }
    cv_.notify_one();
}

inline void AsyncTaskQueue::WorkerLoop() {
    while (true) {
        Task task;
        {
            std::unique_lock<std::mutex> lock(mutex_);
            cv_.wait(lock, [this] { return !running_ || !tasks_.empty(); });
            if (!running_ && tasks_.empty()) {
                return;
            }
            task = std::move(tasks_.front());
            tasks_.pop();
        }
        if (task) {
            task();
        }
    }
}

} // namespace ant::ai_bridge
