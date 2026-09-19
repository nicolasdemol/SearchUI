#include "Papyrus.h"
#include <SearchUI/Search.h>
#include <SearchUI/AsyncSearch.h>
#include <SearchUI/Preferences.h>
using namespace SearchUI;
using namespace RE;
namespace {
    constexpr std::string_view PapyrusClass = "SearchAPI";
    SearchSession& Session() { return AsyncSearch::Get().Session(); }
    std::int32_t GetSearchCount(StaticFunctionTag*) { return static_cast<std::int32_t>(Session().Count()); }
    BSFixedString GetSearchResultName(StaticFunctionTag*, std::int32_t index) { return Session().At(index).name.c_str(); }
    // Compatibility shims for scripts/suspended calls from earlier builds.
    // Removed plugin/phrase settings cannot affect the search engine.
    BSFixedString GetSearchResultPlugin(StaticFunctionTag*, std::int32_t) { return ""; }
    TESForm* GetSearchResult(StaticFunctionTag*, std::int32_t index) {
        const auto result = Session().At(index);
        return result.formID ? TESForm::LookupByID(result.formID) : nullptr;
    }
    void RunSearch(StaticFunctionTag*, BSFixedString term, bool, std::uint32_t mask) {
        AsyncSearch::Get().RunSearch(term.c_str(), mask);
    }
    void StartAsyncSearch(StaticFunctionTag*, BSFixedString term, bool, std::uint32_t mask) {
        AsyncSearch::Get().QueueSearch(term.c_str(), mask);
    }
    void StartFilteredSearch(StaticFunctionTag*, BSFixedString term, bool, std::uint32_t mask,
                             BSFixedString, std::int32_t enchantment) {
        AsyncSearch::Get().QueueSearch(term.c_str(), mask, std::clamp(enchantment, 0, 2));
    }
    void StartItemSearch(StaticFunctionTag*, BSFixedString term, std::uint32_t mask, std::int32_t enchantment) {
        AsyncSearch::Get().QueueSearch(term.c_str(), mask, std::clamp(enchantment, 0, 2));
    }
    bool IsSearchFinished(StaticFunctionTag*) { return Session().GetStatus() != SearchSession::Status::running; }
    bool DidSearchFail(StaticFunctionTag*) { return Session().GetStatus() == SearchSession::Status::failed; }
    void CancelSearch(StaticFunctionTag*) { AsyncSearch::Get().Cancel(); }
    void AddResultsToContainer(StaticFunctionTag*, TESObjectREFR* container, std::int32_t maxResults, std::int32_t quantity) {
        Search::AddSearchResultsToContainer(container, maxResults, quantity);
    }
    bool LoadPreferences(StaticFunctionTag*) { return UserPreferences().Load(); }
    std::int32_t GetPreference(StaticFunctionTag*, BSFixedString key, std::int32_t fallback) {
        if (key == "smart") return 1;
        if (!UserPreferences().HasSaved()) return fallback;
        return UserPreferences().Get().GetInt(key.c_str(), fallback);
    }
    BSFixedString GetPluginFilter(StaticFunctionTag*) { return ""; }
    bool SavePreferences(StaticFunctionTag*, std::int32_t key, bool, std::int32_t maxResults,
                          std::int32_t quantity, std::int32_t mask, BSFixedString, std::int32_t enchantment) {
        return UserPreferences().Save({key, maxResults, quantity, mask, enchantment});
    }
    bool SaveSettings(StaticFunctionTag*, std::int32_t key, std::int32_t maxResults,
                      std::int32_t quantity, std::int32_t mask, std::int32_t enchantment) {
        return UserPreferences().Save({key, maxResults, quantity, mask, enchantment});
    }
}
bool SearchUI::RegisterPapyrusFuncs(RE::BSScript::IVirtualMachine* vm) {
    if (!vm) return false;
    vm->RegisterFunction("RunSearch", PapyrusClass, RunSearch);
    vm->RegisterFunction("StartAsyncSearch", PapyrusClass, StartAsyncSearch);
    vm->RegisterFunction("StartFilteredSearch", PapyrusClass, StartFilteredSearch);
    vm->RegisterFunction("StartItemSearch", PapyrusClass, StartItemSearch);
    vm->RegisterFunction("IsSearchFinished", PapyrusClass, IsSearchFinished);
    vm->RegisterFunction("DidSearchFail", PapyrusClass, DidSearchFail);
    vm->RegisterFunction("CancelSearch", PapyrusClass, CancelSearch);
    vm->RegisterFunction("GetSearchCount", PapyrusClass, GetSearchCount);
    vm->RegisterFunction("GetSearchResultName", PapyrusClass, GetSearchResultName);
    vm->RegisterFunction("GetSearchResultPlugin", PapyrusClass, GetSearchResultPlugin);
    vm->RegisterFunction("GetSearchResult", PapyrusClass, GetSearchResult);
    vm->RegisterFunction("AddResultsToContainer", PapyrusClass, AddResultsToContainer);
    vm->RegisterFunction("LoadPreferences", PapyrusClass, LoadPreferences);
    vm->RegisterFunction("GetPreference", PapyrusClass, GetPreference);
    vm->RegisterFunction("GetPluginFilter", PapyrusClass, GetPluginFilter);
    vm->RegisterFunction("SavePreferences", PapyrusClass, SavePreferences);
    vm->RegisterFunction("SaveSettings", PapyrusClass, SaveSettings);
    logger::info("SearchAPI: registered 17 native functions (including legacy compatibility shims)");
    return true;
}
