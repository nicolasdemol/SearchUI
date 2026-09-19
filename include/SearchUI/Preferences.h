#pragma once
#include <filesystem>
#include <mutex>
#include <string>
#include <string_view>
namespace SearchUI {
    struct Preferences {
        int hotkey = 62;
        int maxResults = 1000;
        int quantity = 50;
        int categories = 107;
        int enchantment = 0;
        bool operator==(const Preferences&) const = default;
        void Validate() const;
        int GetInt(std::string_view key, int fallback) const;
    };
    class PreferenceStore {
    public:
        explicit PreferenceStore(std::filesystem::path path) : _path(std::move(path)) {}
        bool Load(); // Missing/invalid file preserves current values.
        bool Save(const Preferences& value);
        Preferences Get() const { std::lock_guard lock(_mutex); return _value; }
        bool HasSaved() const { std::lock_guard lock(_mutex); return _saved; }
    private:
        std::filesystem::path _path;
        mutable std::mutex _mutex;
        Preferences _value;
        bool _saved = false;
    };
    PreferenceStore& UserPreferences();
}
