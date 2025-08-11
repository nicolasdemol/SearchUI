#include "Papyrus.h"

#include <Sample/Search.h>
#include <Sample/AsyncSearch.h>

using namespace Sample;
using namespace RE;
using namespace RE::BSScript;
using namespace REL;
using namespace SKSE;

namespace {
    constexpr std::string_view PapyrusClass = "SearchAPI";
    // Fonction Papyrus : int GetSearchCount()
    int32_t GetSearchCount(StaticFunctionTag*) { return static_cast<int32_t>(GetLastResults().size()); }

    // Fonction Papyrus : string GetSearchResultName(int index)
    BSFixedString GetSearchResultName(StaticFunctionTag*, int32_t index) {
        if (index >= 0 && index < static_cast<std::int32_t>(GetLastResults().size())) {
            return GetLastResults()[index].name.c_str();
        }
        return "";
    }

    RE::TESForm* GetSearchResult(StaticFunctionTag*, int32_t index) {
        if (index >= 0 && index < static_cast<std::int32_t>(GetLastResults().size())) {
            return RE::TESForm::LookupByID(GetLastResults()[index].formID);
        }
        return nullptr;
    }

    // Fonction Papyrus : void RunSearch(string term, bool exactMatch)
    void RunSearch(StaticFunctionTag*, BSFixedString term, bool exactMatch, uint32_t categoryMask) {
        Search::FindFormsByName(term.c_str(), exactMatch, categoryMask);
    }

    void AddResultsToContainer(StaticFunctionTag*, RE::TESObjectREFR* container, std::int32_t maxResults,
                               std::int32_t consumableQty) {
        Search::AddSearchResultsToContainer(container, maxResults, consumableQty);
    }

    void StartAsyncSearch(StaticFunctionTag*, BSFixedString term, bool exactMatch, uint32_t categoryMask) {
        // Vider les anciens résultats immédiatement
        SetLastResults({});

        // Lancer la recherche asynchrone
        AsyncSearch::Get().QueueSearch(term.c_str(), exactMatch, categoryMask,
                                       [](std::vector<Search::Result> results) { SetLastResults(std::move(results)); });
    }


    bool IsSearchFinished(StaticFunctionTag*) {
        // On suppose qu'une recherche est prête si GetLastResults() n'est pas vide
        return !GetLastResults().empty();
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
    vm->RegisterFunction("StartAsyncSearch", PapyrusClass, StartAsyncSearch);
    vm->RegisterFunction("IsSearchFinished", PapyrusClass, IsSearchFinished);

    vm->RegisterFunction("GetSearchCount", PapyrusClass, GetSearchCount);
    vm->RegisterFunction("GetSearchResultName", PapyrusClass, GetSearchResultName);
    vm->RegisterFunction("GetSearchResult", PapyrusClass, GetSearchResult);
    vm->RegisterFunction("AddResultsToContainer", PapyrusClass, AddResultsToContainer);

    return true;
}