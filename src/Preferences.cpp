#include <SearchUI/Preferences.h>
#include <yaml-cpp/yaml.h>
#include <spdlog/spdlog.h>
#include <Windows.h>
#include <fstream>
#include <stdexcept>

namespace SearchUI {
    int Preferences::GetInt(std::string_view key, int fallback) const {
        // Papyrus/BSFixedString keys are case-insensitive. The string pool may
        // return another spelling (e.g. "Enchantment") than the script literal.
        std::string name(key);
        for (auto& c : name) {
            if (c >= 'A' && c <= 'Z') c = static_cast<char>(c + ('a' - 'A'));
        }
        if (name == "smart") return 1;
        if (name == "hotkey") return hotkey;
        if (name == "maxresults") return maxResults;
        if (name == "quantity") return quantity;
        if (name == "categories") return categories;
        if (name == "enchantment") return enchantment;
        return fallback;
    }

    void Preferences::Validate() const {
        if (hotkey < 1 || hotkey > 281 || maxResults < 10 || maxResults > 5000 ||
            quantity < 1 || quantity > 500 || categories < 0 || categories > 255 || enchantment < 0 || enchantment > 2)
            throw std::invalid_argument("SearchUI preference outside allowed range");
    }
    PreferenceStore& UserPreferences() {
        static PreferenceStore store("Data/SKSE/Plugins/SearchUI.settings.yaml");
        return store;
    }
    bool PreferenceStore::Load() {
        std::lock_guard lock(_mutex);
        try {
            if (!std::filesystem::exists(_path)) {
                spdlog::info("Global preferences file missing: {}; using {}", std::filesystem::absolute(_path).string(),
                             _saved ? "last loaded settings" : "defaults");
                return false;
            }
            const auto node = YAML::LoadFile(_path.string());
            const auto version = node["version"].as<int>();
            if (version != 1 && version != 2) throw std::runtime_error("Unsupported preference schema");
            // Schema 1's smart/plugin values are deliberately ignored on upgrade.
            Preferences value;
            value.hotkey = node["hotkey"].as<int>();
            value.maxResults = node["maxResults"].as<int>();
            value.quantity = node["quantity"].as<int>();
            value.categories = node["categories"].as<int>();
            value.enchantment = node["enchantment"].as<int>();
            value.Validate();
            _value = std::move(value); _saved = true;
            spdlog::info("Global preferences loaded: {}; hotkey={}, maxResults={}, quantity={}, categories={}, enchantment={}",
                         std::filesystem::absolute(_path).string(), _value.hotkey, _value.maxResults,
                         _value.quantity, _value.categories, _value.enchantment);
            return true;
        } catch (const std::exception& e) {
            spdlog::warn("SearchUI preferences not loaded: {}", e.what());
            return false;
        }
    }
    bool PreferenceStore::Save(const Preferences& value) {
        std::lock_guard lock(_mutex);
        auto temporary = _path;
        temporary += L".tmp";
        try {
            value.Validate();
            YAML::Node node;
            node["version"] = 2;
            node["hotkey"] = value.hotkey;
            node["maxResults"] = value.maxResults;
            node["quantity"] = value.quantity;
            node["categories"] = value.categories;
            node["enchantment"] = value.enchantment;
            std::filesystem::create_directories(_path.parent_path());
            { std::ofstream stream(temporary, std::ios::trunc | std::ios::binary);
              stream.exceptions(std::ios::failbit | std::ios::badbit);
              stream << node; stream.close(); }
            if (!MoveFileExW(temporary.c_str(), _path.c_str(), MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH))
                throw std::runtime_error("Cannot replace settings file, Windows error " + std::to_string(GetLastError()));
            _value = value; _saved = true;
            spdlog::info("Global preferences saved: {}; hotkey={}, maxResults={}, quantity={}, categories={}, enchantment={}",
                         std::filesystem::absolute(_path).string(), _value.hotkey, _value.maxResults,
                         _value.quantity, _value.categories, _value.enchantment);
            return true;
        } catch (const std::exception& e) {
            std::error_code ignored;
            std::filesystem::remove(temporary, ignored);
            spdlog::error("SearchUI preferences not saved: {}", e.what());
            return false;
        }
    }
}
