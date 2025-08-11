#pragma once

#include <string>
#include <vector>

namespace Sample {
    class Search {
    public:
        struct Result {
            std::string name;
            std::uint32_t formID;
        };

        enum FormCategory : std::uint32_t {
            Weapons = 1 << 0,
            Armors = 1 << 1,
            Books = 1 << 2,
            Potions = 1 << 3,
            MiscItems = 1 << 4,
            Ingredients = 1 << 5,
            Ammo = 1 << 6,
            Lights = 1 << 7,
            All = 0xFFFFFFFF
        };


        static std::vector<Result> FindFormsByName(const std::string& searchTerm, bool exactMatch = false,
                                                   std::uint32_t categoryMask = FormCategory::All);
        static void AddSearchResultsToContainer(RE::TESObjectREFR* container, std::int32_t maxResults,
                                                std::int32_t consumableQty);
    };
    
    std::vector<Search::Result>& GetLastResults();
    void SetLastResults(std::vector<Search::Result>&& results);
}  // namespace Sample
