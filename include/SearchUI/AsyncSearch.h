#pragma once
#include <SearchUI/SearchCore.h>
#include <condition_variable>
#include <optional>
#include <thread>
namespace SearchUI {
    class AsyncSearch {
    public:
        static AsyncSearch& Get();
        void QueueSearch(const std::string& term, std::uint32_t categories, int enchantment = 0);
        void RunSearch(const std::string& term, std::uint32_t categories);
        void Cancel();
        SearchSession& Session() { return _session; }
    private:
        struct Job {
            std::uint64_t generation;
            std::string term;
            std::uint32_t categories;
            int enchantment;
        };
        AsyncSearch();
        ~AsyncSearch();
        void Execute(const Job& job);
        void ThreadLoop();
        SearchSession _session;
        std::mutex _mutex;
        std::condition_variable _condition;
        std::optional<Job> _pending;
        bool _running{true};
        std::thread _worker;
    };
}
