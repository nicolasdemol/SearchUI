#pragma once

#include <Sample/Search.h>

#include <atomic>
#include <condition_variable>
#include <functional>
#include <future>
#include <mutex>
#include <queue>
#include <string>
#include <thread>

namespace Sample {
    class AsyncSearch {
    public:
        using Callback = std::function<void(std::vector<Search::Result>)>;

        static AsyncSearch& Get();

        void QueueSearch(const std::string& term, bool exactMatch, uint32_t categoryMask, Callback onDone);
        void Update();

    private:
        AsyncSearch();
        ~AsyncSearch();

        struct Job {
            std::string term;
            bool exactMatch;
            uint32_t categoryMask;
            Callback callback;
        };

        void ThreadLoop();

        std::thread _worker;
        std::mutex _queueMutex;
        std::condition_variable _condition;
        std::queue<Job> _queue;
        std::atomic<bool> _running;

        std::mutex _resultsMutex;
        std::queue<std::function<void()>> _results;
    };
}  // namespace Sample