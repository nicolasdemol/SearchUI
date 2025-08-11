#include <codecvt>
#include <locale>

#include <Sample/Search.h>
#include <spdlog/spdlog.h>

using namespace Sample;
using namespace SKSE;

namespace Sample {
    static std::vector<Search::Result> g_lastResults;

    std::vector<Search::Result>& GetLastResults() { return g_lastResults; }

    void SetLastResults(std::vector<Search::Result>&& results) { g_lastResults = std::move(results); }

    std::wstring RemoveDiacriticsW(const std::wstring& input) {
        static const std::unordered_map<wchar_t, wchar_t> accentMap = {
            {L'à', L'a'}, {L'â', L'a'}, {L'ä', L'a'}, {L'á', L'a'}, {L'ã', L'a'}, {L'ç', L'c'}, {L'é', L'e'},
            {L'è', L'e'}, {L'ê', L'e'}, {L'ë', L'e'}, {L'î', L'i'}, {L'ï', L'i'}, {L'ì', L'i'}, {L'í', L'i'},
            {L'ô', L'o'}, {L'ö', L'o'}, {L'ò', L'o'}, {L'ó', L'o'}, {L'õ', L'o'}, {L'ù', L'u'}, {L'û', L'u'},
            {L'ü', L'u'}, {L'ú', L'u'}, {L'ÿ', L'y'}, {L'À', L'a'}, {L'Â', L'a'}, {L'Ä', L'a'}, {L'Á', L'a'},
            {L'Ã', L'a'}, {L'Ç', L'c'}, {L'É', L'e'}, {L'È', L'e'}, {L'Ê', L'e'}, {L'Ë', L'e'}, {L'Î', L'i'},
            {L'Ï', L'i'}, {L'Ì', L'i'}, {L'Í', L'i'}, {L'Ô', L'o'}, {L'Ö', L'o'}, {L'Ò', L'o'}, {L'Ó', L'o'},
            {L'Õ', L'o'}, {L'Ù', L'u'}, {L'Û', L'u'}, {L'Ü', L'u'}, {L'Ú', L'u'}, {L'Ÿ', L'y'}};

        std::wstring output;
        for (wchar_t ch : input) {
            auto it = accentMap.find(ch);
            output += (it != accentMap.end()) ? it->second : ch;
        }
        return output;
    }

    std::string RemoveDiacritics(const std::string& input) {
        try {
            std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
            std::wstring wide = converter.from_bytes(input);
            std::wstring cleaned = RemoveDiacriticsW(wide);
            return converter.to_bytes(cleaned);
        } catch (const std::range_error& e) {
            log::error("RemoveDiacritics: range_error caught for input '{}': {}", input, e.what());
            return input;  // fallback: return unchanged
        } catch (const std::exception& e) {
            log::error("RemoveDiacritics: exception caught for input '{}': {}", input, e.what());
            return input;
        }
    }


    std::string Trim(const std::string& str) {
        const auto start = str.find_first_not_of(" \t\n\r\f\v");
        if (start == std::string::npos) return "";
        const auto end = str.find_last_not_of(" \t\n\r\f\v");
        return str.substr(start, end - start + 1);
    }

    std::string NormalizeSearchTerm(const std::string& term) {
        std::string normalized = RemoveDiacritics(term);
        normalized = Trim(normalized);
        std::transform(normalized.begin(), normalized.end(), normalized.begin(), ::tolower);
        return normalized;
    }


    template <typename T>
    void SearchForms(const std::string& normalizedSearchTerm, bool exactMatch,
                     std::vector<Search::Result>& outResults) {
        auto* dataHandler = RE::TESDataHandler::GetSingleton();
        if (!dataHandler) return;

        const auto& forms = dataHandler->GetFormArray<T>();
        for (const auto* form : forms) {
            if (!form) continue;

            std::string name = form->GetName();
            if (name.empty()) continue;
            
            std::string nameLower = NormalizeSearchTerm(name);

            bool match = false;
            if (exactMatch) {
                match = nameLower.find(normalizedSearchTerm) != std::string::npos;
            } else {
                std::istringstream iss(normalizedSearchTerm);
                std::string word;
                match = true;
                while (iss >> word) {
                    if (nameLower.find(word) == std::string::npos) {
                        match = false;
                        break;
                    }
                }
            }
            
            if (match) {
                outResults.push_back({name, form->GetFormID()});
            }
        }
    }
}  // namespace

void Search::AddSearchResultsToContainer(RE::TESObjectREFR* container, std::int32_t maxResults,
                                         std::int32_t consumableQty) {
    if (!container) {
        log::error("Container is null in AddSearchResultsToContainer");
        return;
    }

    std::int32_t count = 0;
    for (const auto& result : GetLastResults()) {
        if (count >= maxResults) break;

        auto* form = RE::TESForm::LookupByID(result.formID);
        if (!form) {
            log::warn("Skipping null formID {:08X}", result.formID);
            continue;
        }

        auto* boundObject = form->As<RE::TESBoundObject>();
        if (!boundObject) {
            log::warn("Skipping formID {:08X} — not a TESBoundObject", result.formID);
            continue;
        }

        std::int32_t quantity = 1;
        RE::FormType formType = boundObject->GetFormType();
        if (formType == RE::FormType::Ammo || formType == RE::FormType::Ingredient || formType == RE::FormType::Misc) {
            quantity = consumableQty;
        }

        try {
            container->AddObjectToContainer(boundObject, nullptr, quantity, nullptr);
        } catch (...) {
            log::error("Crash prevented while injecting {:08X}", result.formID);
        }
        ++count;
    }
}



std::vector<Search::Result> Search::FindFormsByName(const std::string& searchTerm, bool exactMatch,
                                                    std::uint32_t categoryMask) {
    std::vector<Result> results;

    std::string normalizedSearchTerm = NormalizeSearchTerm(searchTerm);

    if (categoryMask & Weapons) SearchForms<RE::TESObjectWEAP>(normalizedSearchTerm, exactMatch, results);
    if (categoryMask & Armors) SearchForms<RE::TESObjectARMO>(normalizedSearchTerm, exactMatch, results);
    if (categoryMask & Books) SearchForms<RE::TESObjectBOOK>(normalizedSearchTerm, exactMatch, results);
    if (categoryMask & Potions) SearchForms<RE::AlchemyItem>(normalizedSearchTerm, exactMatch, results);
    if (categoryMask & MiscItems) SearchForms<RE::TESObjectMISC>(normalizedSearchTerm, exactMatch, results);
    if (categoryMask & Ingredients) SearchForms<RE::IngredientItem>(normalizedSearchTerm, exactMatch, results);
    if (categoryMask & Ammo) SearchForms<RE::TESAmmo>(normalizedSearchTerm, exactMatch, results);
    if (categoryMask & Lights) SearchForms<RE::TESObjectLIGH>(normalizedSearchTerm, exactMatch, results);

    log::info("Search for '{}' returned {} result(s)", searchTerm, results.size());
    SetLastResults(std::move(results));
    return GetLastResults();
}