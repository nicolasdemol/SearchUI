#pragma once
#include <algorithm>
#include <cstdint>
#include <functional>
#include <mutex>
#include <string>
#include <vector>

namespace SearchUI {
    struct SearchResult {
        std::string name;
        std::uint32_t formID{};
    };
    struct IndexedItem {
        SearchResult result;
        std::string key;
        std::uint32_t category{};
        bool enchanted{};
    };
    using SearchIndex = std::vector<IndexedItem>;
    std::string NormalizeSearchTerm(const std::string& text);
    std::vector<SearchResult> FilterIndex(const SearchIndex& index, const std::string& term,
        std::uint32_t categories, int enchantment,
        const std::function<bool()>& cancelled = {});

    // Generation and publication share one lock. A late worker cannot replace
    // a newer request, including after cancelling or loading another save.
    class SearchSession {
    public:
        enum class Status { idle, running, complete, failed, cancelled };
        std::uint64_t Begin() {
            std::lock_guard lock(_mutex);
            _results.clear(); _status = Status::running;
            return ++_generation;
        }
        void Finish(std::uint64_t generation, std::vector<SearchResult> results, bool failed = false) {
            std::lock_guard lock(_mutex);
            if (generation != _generation || _status != Status::running) return;
            _results = std::move(results);
            _status = failed ? Status::failed : Status::complete;
        }
        void Cancel() {
            std::lock_guard lock(_mutex);
            ++_generation; _results.clear(); _status = Status::cancelled;
        }
        bool Current(std::uint64_t generation) const {
            std::lock_guard lock(_mutex);
            return generation == _generation && _status == Status::running;
        }
        Status GetStatus() const { std::lock_guard lock(_mutex); return _status; }
        std::vector<SearchResult> Results() const { std::lock_guard lock(_mutex); return _results; }
        std::size_t Count() const { std::lock_guard lock(_mutex); return _results.size(); }
        SearchResult At(std::int32_t index) const {
            std::lock_guard lock(_mutex);
            return index >= 0 && static_cast<std::size_t>(index) < _results.size() ? _results[index] : SearchResult{};
        }
    private:
        mutable std::mutex _mutex;
        std::uint64_t _generation{};
        Status _status{Status::idle};
        std::vector<SearchResult> _results;
    };
}
