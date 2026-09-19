#include <SearchUI/SearchCore.h>
#include <SearchUI/Preferences.h>
#include <chrono>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <thread>
using namespace SearchUI;
namespace {
    int checks = 0;
    void Check(bool ok, const char* message) { ++checks; if (!ok) throw std::runtime_error(message); }
    IndexedItem Item(std::string name, std::uint32_t id, std::uint32_t category, bool enchanted) {
        return {{name, id}, NormalizeSearchTerm(name), category, enchanted};
    }
}
int main() {
    try {
        const SearchIndex index{
            Item("Épée d'acier", 1, 1, false),
            Item("Épée de feu", 2, 1, true),
            Item("Steel Armor", 3, 2, false),
            Item("龙骨剑", 4, 1, false),
            Item("СТАЛЬНОЙ МЕЧ", 5, 1, false)
        };
        Check(FilterIndex(index, "epee", 255, 0).size() == 2, "Latin accent folding");
        Check(FilterIndex(index, "e\xCC\x81pee", 255, 0).size() == 2, "Decomposed accent folding");
        Check(FilterIndex(index, "acier epee", 255, 0).at(0).formID == 1, "Unordered smart search is always active");
        Check(FilterIndex(index, "epee acier", 255, 0).size() == 1, "All words match regardless of order");
        Check(FilterIndex(index, "epee missing", 255, 0).empty(), "Every search word is required");
        Check(FilterIndex(index, "steel", 1, 0).empty(), "Category filter excludes armor");
        Check(FilterIndex(index, "steel", 2, 0).at(0).formID == 3, "Category filter includes armor");
        Check(FilterIndex(index, "epee", 0, 0).empty(), "No categories");
        Check(FilterIndex(index, "epee", 255, 1).at(0).formID == 1, "Exclude enchanted");
        Check(FilterIndex(index, "epee", 255, 2).at(0).formID == 2, "Only enchanted");
        Check(FilterIndex(index, "missing", 255, 0).empty(), "Zero results");
        Check(FilterIndex(index, "  ", 255, 0).empty(), "Blank query does not enumerate everything");
        Check(FilterIndex(index, "龙骨", 255, 0).at(0).formID == 4, "Chinese text survives");
        Check(FilterIndex(index, "стальной", 255, 0).at(0).formID == 5, "Cyrillic case folding");
        Check(NormalizeSearchTerm("ガ") == NormalizeSearchTerm("カ\xE3\x82\x99"), "Kana normalization");
        Check(NormalizeSearchTerm("ガ") != NormalizeSearchTerm("カ"), "Kana marks retained");
        Check(NormalizeSearchTerm("😀") == "😀", "Surrogate pair preserved");
        Check(FilterIndex(index, "epee", 255, 0, [] { return true; }).empty(), "Cancellation");
        bool invalid = false;
        try { NormalizeSearchTerm(std::string(1, char(0xFF))); } catch (const std::invalid_argument&) { invalid = true; }
        Check(invalid, "Invalid UTF-8 rejected");

        SearchSession session;
        auto request = session.Begin();
        Check(session.GetStatus() == SearchSession::Status::running, "Running state");
        session.Finish(request, {});
        Check(session.GetStatus() == SearchSession::Status::complete && session.Count() == 0, "Empty search completes");
        auto old = session.Begin();
        auto current = session.Begin();
        session.Finish(old, {{"old", 1}});
        Check(session.Count() == 0 && session.GetStatus() == SearchSession::Status::running, "Reject stale completion");
        session.Finish(current, {{"new", 2}});
        auto snapshot = session.Results();
        session.Cancel();
        Check(snapshot.at(0).formID == 2 && session.Count() == 0, "Results are independent snapshots");
        session.Finish(current, {{"late", 3}});
        Check(session.Count() == 0, "Cancel rejects in-flight result");
        Check(session.At(-1).formID == 0 && session.At(999).formID == 0, "Bounds checks");
        request = session.Begin(); session.Finish(request, {}, true);
        Check(session.GetStatus() == SearchSession::Status::failed, "Failure is distinct from empty success");
        std::thread writer([&] { for (int i=0; i<3000; ++i) { auto id=session.Begin(); session.Finish(id, {{"item", 7}}); } });
        for (int i=0; i<3000; ++i) { (void)session.Results(); (void)session.At(0); session.Cancel(); }
        writer.join();
        current=session.Begin(); session.Finish(current, {{"final", 42}});
        Check(session.At(0).formID == 42, "Concurrent read, publish and cancel stress");

        const auto folder = std::filesystem::temp_directory_path() / ("SearchUI-test-" +
            std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
        const auto path = folder / "settings.yaml";
        PreferenceStore preferences(path);
        Check(!preferences.Load(), "Missing preferences preserve defaults");
        Check(!preferences.HasSaved() && preferences.Get() == Preferences{}, "First run has fixed global defaults");
        Preferences value{63, 250, 25, 131, 2};
        Check(preferences.Save(value), "Preferences save");
        PreferenceStore reloaded(path);
        Check(reloaded.Load() && reloaded.Get() == value, "All remaining preferences round trip");
        for (int mode = 0; mode <= 2; ++mode) {
            auto selected = value;
            selected.enchantment = mode;
            Check(preferences.Save(selected) && reloaded.Load(), "Each enchantment mode persists");
            for (const auto* key : {"enchantment", "Enchantment", "ENCHANTMENT", "eNcHaNtMeNt"}) {
                Check(reloaded.Get().GetInt(key, -1) == mode, "Papyrus key casing cannot reset enchantment selection");
            }
        }
        Check(reloaded.Get().GetInt("HOTKEY", -1) == value.hotkey &&
              reloaded.Get().GetInt("MaxResults", -1) == value.maxResults &&
              reloaded.Get().GetInt("Quantity", -1) == value.quantity &&
              reloaded.Get().GetInt("CATEGORIES", -1) == value.categories,
              "Other preference keys also ignore Papyrus casing");
        Check(reloaded.Get().GetInt("SMART", 0) == 1 && reloaded.Get().GetInt("unknown", -123) == -123,
              "Legacy smart key and unknown-key fallback");
        auto bad=value; bad.maxResults=0;
        Check(!reloaded.Save(bad), "Invalid preferences rejected");
        Check(reloaded.Load() && reloaded.Get() == value, "Failed write preserves previous file");
        { std::ofstream legacy(path); legacy << "version: 1\nhotkey: 63\nsmart: 0\nmaxResults: 250\nquantity: 25\ncategories: 131\nenchantment: 2\nplugin: Weapons.esl\n"; }
        Check(preferences.Load() && preferences.Get() == value, "Legacy disabled smart search and plugin filter are ignored");
        Check(FilterIndex(index, "acier epee", preferences.Get().categories, 0).size() == 1,
              "Smart matching stays active after loading legacy preferences");
        Check(preferences.Save(value), "Legacy preferences rewritten successfully");
        { std::ifstream stream(path); const std::string yaml((std::istreambuf_iterator<char>(stream)), {});
          Check(yaml.find("version: 2") != std::string::npos && yaml.find("smart:") == std::string::npos &&
                yaml.find("plugin:") == std::string::npos, "Schema 2 drops removed settings"); }
        value.quantity=30;
        Check(reloaded.Save(value) && preferences.Load() && preferences.Get() == value, "Atomic replacement");
        value.hotkey=64; value.maxResults=350; value.quantity=40; value.categories=255; value.enchantment=1;
        Check(preferences.Save(value) && reloaded.Load() && reloaded.Get() == value,
              "A stale reader reloads every current global setting");
        PreferenceStore restarted(path);
        Check(restarted.Load() && restarted.Get() == value, "Fresh session restores global settings without any game save");
        { std::ofstream corrupt(path); corrupt << "version: [broken"; }
        Check(!preferences.Load() && preferences.Get() == value, "Malformed file leaves active settings intact");
        std::filesystem::remove(path); // Only the two paths created by this test.
        Check(!preferences.Load() && preferences.HasSaved() && preferences.Get() == value,
              "Missing file preserves last global settings in memory, not save-specific values");
        std::filesystem::remove(folder);
        std::cout << "PASS: " << checks << " checks (search, Unicode, request lifecycle, concurrency, preferences)\n";
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "FAIL: " << e.what() << '\n'; return 1;
    }
}
