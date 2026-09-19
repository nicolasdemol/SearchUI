#include <SearchUI/AsyncSearch.h>
#include <SearchUI/Search.h>
using namespace SearchUI;
AsyncSearch& AsyncSearch::Get() { static AsyncSearch instance; return instance; }
AsyncSearch::AsyncSearch() : _worker(&AsyncSearch::ThreadLoop, this) {}
AsyncSearch::~AsyncSearch() {
    { std::lock_guard lock(_mutex); _running = false; _pending.reset(); _session.Cancel(); }
    _condition.notify_one();
    if (_worker.joinable()) _worker.join();
}
void AsyncSearch::Cancel() {
    std::lock_guard lock(_mutex);
    _pending.reset(); _session.Cancel();
}
void AsyncSearch::QueueSearch(const std::string& term, std::uint32_t categories, int enchantment) {
    { std::lock_guard lock(_mutex);
      _pending = Job{_session.Begin(), term, categories, enchantment}; }
    _condition.notify_one();
}
void AsyncSearch::RunSearch(const std::string& term, std::uint32_t categories) {
    Job job;
    { std::lock_guard lock(_mutex); _pending.reset(); job = {_session.Begin(), term, categories, 0}; }
    Execute(job);
}
void AsyncSearch::Execute(const Job& job) {
    try {
        auto index = Search::GetIndex();
        if (!index) throw std::runtime_error("Search index is not ready; wait for DataLoaded");
        auto results = FilterIndex(*index, job.term, job.categories, job.enchantment,
                                  [&] { return !_session.Current(job.generation); });
        const auto count = results.size();
        _session.Finish(job.generation, std::move(results));
        logger::info("Search request {} completed: {} result(s)", job.generation, count);
    } catch (const std::exception& e) {
        _session.Finish(job.generation, {}, true);
        logger::error("Search request {} failed: {}", job.generation, e.what());
    }
}
void AsyncSearch::ThreadLoop() {
    for (;;) {
        Job job;
        { std::unique_lock lock(_mutex);
          _condition.wait(lock, [&] { return !_running || _pending.has_value(); });
          if (!_running) return;
          job = std::move(*_pending); _pending.reset(); }
        Execute(job);
    }
}
