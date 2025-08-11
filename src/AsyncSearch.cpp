
#include <Sample/AsyncSearch.h>
#include <Sample/Search.h>
#include <spdlog/spdlog.h>

using namespace Sample;

AsyncSearch& AsyncSearch::Get() {
    static AsyncSearch instance;
    return instance;
}

AsyncSearch::AsyncSearch() : _running(true), _worker(&AsyncSearch::ThreadLoop, this) {}

AsyncSearch::~AsyncSearch() {
    _running = false;
    _condition.notify_one();
    if (_worker.joinable()) _worker.join();
}

void AsyncSearch::QueueSearch(const std::string& term, bool exactMatch, uint32_t categoryMask, Callback onDone) {
    {
        std::lock_guard lock(_queueMutex);
        _queue.push({term, exactMatch, categoryMask, onDone});
    }
    _condition.notify_one();
}

void AsyncSearch::Update() {
    std::queue<std::function<void()>> ready;
    {
        std::lock_guard lock(_resultsMutex);
        std::swap(ready, _results);
    }
    while (!ready.empty()) {
        ready.front()();
        ready.pop();
    }
}

void AsyncSearch::ThreadLoop() {
    while (_running) {
        Job job;
        {
            std::unique_lock lock(_queueMutex);
            _condition.wait(lock, [&] { return !_queue.empty() || !_running; });
            if (!_running) break;
            job = std::move(_queue.front());
            _queue.pop();
        }

        auto results = Search::FindFormsByName(job.term, job.exactMatch, job.categoryMask);

        {
            std::lock_guard lock(_resultsMutex);
            _results.push([results = std::move(results), cb = job.callback]() mutable { cb(std::move(results)); });
        }
    }
}
