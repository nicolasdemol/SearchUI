#include "Papyrus.h"

#include <Sample/Search.h>

using namespace Sample;
using namespace RE;
using namespace RE::BSScript;
using namespace REL;
using namespace SKSE;

namespace {
    constexpr std::string_view PapyrusClass = "SearchAPI";
    static std::vector<Search::Result> g_lastResults;

    std::vector<Search::Result>& GetLastResults() { return g_lastResults; }

    // Fonction Papyrus : int GetSearchCount()
    int32_t GetSearchCount(StaticFunctionTag*) { return static_cast<int32_t>(g_lastResults.size()); }

    // Fonction Papyrus : string GetSearchResultName(int index)
    BSFixedString GetSearchResultName(StaticFunctionTag*, int32_t index) {
        if (index >= 0 && index < static_cast<std::int32_t>(g_lastResults.size())) {
            return g_lastResults[index].name.c_str();
        }
        return "";
    }

    RE::TESForm* GetSearchResult(StaticFunctionTag*, int32_t index) {
        if (index >= 0 && index < static_cast<std::int32_t>(g_lastResults.size())) {
            return RE::TESForm::LookupByID(g_lastResults[index].formID);
        }
        return nullptr;
    }

    // Fonction Papyrus : void RunSearch(string term, bool exactMatch)
    void RunSearch(StaticFunctionTag*, BSFixedString term, bool exactMatch) {
        g_lastResults = Search::FindFormsByName(term.c_str(), exactMatch);
    }
}  // namespace Sample

/**
 * This is the function that acts as a registration callback for Papyrus functions. Within you can register functions
 * to bind to native code. The first argument of such bindings is the function name, the second is the class name, and
 * third is the function that will be invoked in C++ to handle it. The callback should return <code>true</code> on
 * success.
 */
bool Sample::RegisterPapyrusFuncs(IVirtualMachine* vm) {
    vm->RegisterFunction("RunSearch", PapyrusClass, RunSearch);
    vm->RegisterFunction("GetSearchCount", PapyrusClass, GetSearchCount);
    vm->RegisterFunction("GetSearchResultName", PapyrusClass, GetSearchResultName);
    vm->RegisterFunction("GetSearchResult", PapyrusClass, GetSearchResult);
    return true;
}