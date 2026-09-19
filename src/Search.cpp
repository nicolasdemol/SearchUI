#include <SearchUI/Search.h>
#include <SearchUI/AsyncSearch.h>
using namespace SearchUI;
namespace {
    std::atomic<std::shared_ptr<const SearchIndex>> index;
    template <class T>
    void IndexForms(RE::TESDataHandler& data, SearchIndex& records, std::uint32_t category) {
        for (const auto* form : data.GetFormArray<T>()) {
            if (!form) continue;
            const auto* name = form->GetName();
            if (!name || !*name) continue;
            const auto* enchantable = form->template As<RE::TESEnchantableForm>();
            try {
                records.push_back({{name, form->GetFormID()}, NormalizeSearchTerm(name),
                    category, enchantable && enchantable->formEnchanting});
            } catch (const std::exception& e) {
                logger::warn("Skipping invalid item text for {:08X}: {}", form->GetFormID(), e.what());
            }
        }
    }
}
void Search::BuildIndex() {
    try {
        auto* data = RE::TESDataHandler::GetSingleton();
        if (!data) throw std::runtime_error("TESDataHandler is null");
        auto records = std::make_shared<SearchIndex>();
        IndexForms<RE::TESObjectWEAP>(*data, *records, Weapons);
        IndexForms<RE::TESObjectARMO>(*data, *records, Armors);
        IndexForms<RE::TESObjectBOOK>(*data, *records, Books);
        IndexForms<RE::AlchemyItem>(*data, *records, Potions);
        IndexForms<RE::TESObjectMISC>(*data, *records, MiscItems);
        IndexForms<RE::IngredientItem>(*data, *records, Ingredients);
        IndexForms<RE::TESAmmo>(*data, *records, Ammo);
        IndexForms<RE::TESObjectLIGH>(*data, *records, Lights);
        std::sort(records->begin(), records->end(), [](const auto& a, const auto& b) {
            return std::tie(a.key, a.result.formID) < std::tie(b.key, b.result.formID);
        });
        logger::info("Search index ready: {} named items", records->size());
        index.store(std::move(records));
    } catch (const std::exception& e) {
        index.store(nullptr);
        logger::error("Cannot build search index: {}", e.what());
    }
}
std::shared_ptr<const SearchIndex> Search::GetIndex() { return index.load(); }
void Search::AddSearchResultsToContainer(RE::TESObjectREFR* container, std::int32_t maxResults, std::int32_t quantity) {
    if (!container || maxResults <= 0) return;
    quantity = std::clamp(quantity, 1, 500);
    const auto results = AsyncSearch::Get().Session().Results();
    std::int32_t count = 0;
    for (const auto& result : results) {
        if (count >= maxResults) break;
        auto* form = RE::TESForm::LookupByID<RE::TESBoundObject>(result.formID);
        if (!form) continue;
        const auto type = form->GetFormType();
        const bool stack = type == RE::FormType::Ammo || type == RE::FormType::Ingredient || type == RE::FormType::Misc;
        container->AddObjectToContainer(form, nullptr, stack ? quantity : 1, nullptr);
        ++count;
    }
}
