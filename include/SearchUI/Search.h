#pragma once
#include <SearchUI/SearchCore.h>
#include <memory>
namespace RE { class TESObjectREFR; }
namespace SearchUI {
    class Search {
    public:
        using Result = SearchResult;
        enum FormCategory : std::uint32_t {
            Weapons = 1, Armors = 2, Books = 4, Potions = 8,
            MiscItems = 16, Ingredients = 32, Ammo = 64, Lights = 128, All = 255
        };
        static void BuildIndex(); // Game thread only, at DataLoaded.
        static std::shared_ptr<const SearchIndex> GetIndex();
        static void AddSearchResultsToContainer(RE::TESObjectREFR*, std::int32_t maxResults, std::int32_t quantity);
    };
}
