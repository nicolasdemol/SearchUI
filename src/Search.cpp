#include <Sample/Search.h>
#include <spdlog/spdlog.h>

using namespace Sample;
using namespace SKSE;

namespace {
    std::string RemoveDiacritics(const std::string& input) {
        static const std::unordered_map<char, char> accentMap = {
            {'à', 'a'}, {'â', 'a'}, {'ä', 'a'}, {'á', 'a'}, {'ã', 'a'}, {'ç', 'c'}, {'é', 'e'}, {'è', 'e'},
            {'ê', 'e'}, {'ë', 'e'}, {'î', 'i'}, {'ï', 'i'}, {'ì', 'i'}, {'í', 'i'}, {'ô', 'o'}, {'ö', 'o'},
            {'ò', 'o'}, {'ó', 'o'}, {'õ', 'o'}, {'ù', 'u'}, {'û', 'u'}, {'ü', 'u'}, {'ú', 'u'}, {'ÿ', 'y'},
            {'À', 'a'}, {'Â', 'a'}, {'Ä', 'a'}, {'Á', 'a'}, {'Ã', 'a'}, {'Ç', 'c'}, {'É', 'e'}, {'È', 'e'},
            {'Ê', 'e'}, {'Ë', 'e'}, {'Î', 'i'}, {'Ï', 'i'}, {'Ì', 'i'}, {'Í', 'i'}, {'Ô', 'o'}, {'Ö', 'o'},
            {'Ò', 'o'}, {'Ó', 'o'}, {'Õ', 'o'}, {'Ù', 'u'}, {'Û', 'u'}, {'Ü', 'u'}, {'Ú', 'u'}, {'Ÿ', 'y'}};

        std::string output;
        for (char ch : input) {
            auto it = accentMap.find(ch);
            output += (it != accentMap.end()) ? it->second : ch;
        }
        return output;
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
        std::ranges::transform(normalized, normalized.begin(), ::tolower);
        return normalized;
    }


    template <typename T>
    void SearchForms(const std::string& searchTerm, bool exactMatch, std::vector<Search::Result>& outResults) {
        auto* dataHandler = RE::TESDataHandler::GetSingleton();
        if (!dataHandler) return;

        const auto& forms = dataHandler->GetFormArray<T>();
        for (const auto* form : forms) {
            if (!form) continue;

            std::string name = form->GetName();
            if (name.empty()) continue;

            std::string nameLower = NormalizeSearchTerm(name);
            std::string termLower = NormalizeSearchTerm(searchTerm);

            std::ranges::transform(nameLower, nameLower.begin(), ::tolower);
            std::ranges::transform(termLower, termLower.begin(), ::tolower);

            bool match = exactMatch ? (nameLower == termLower) : (nameLower.find(termLower) != std::string::npos);
            if (match) {
                outResults.push_back({name, form->GetFormID()});
            }
        }
    }
}  // namespace

std::vector<Search::Result> Search::FindFormsByName(const std::string& searchTerm, bool exactMatch) {
    std::vector<Result> results;

    SearchForms<RE::TESObjectWEAP>(searchTerm, exactMatch, results);  // Armes
    SearchForms<RE::TESObjectARMO>(searchTerm, exactMatch, results);  // Armures
    SearchForms<RE::TESObjectBOOK>(searchTerm, exactMatch, results);  // Livres
    SearchForms<RE::TESObjectMISC>(searchTerm, exactMatch, results);  // Objets divers
    SearchForms<RE::AlchemyItem>(searchTerm, exactMatch, results);    // Potions/poisons
    SearchForms<RE::TESAmmo>(searchTerm, exactMatch, results);        // Flèches et carreaux
    SearchForms<RE::IngredientItem>(searchTerm, exactMatch, results);  // Ingrédients

    log::info("Search for '{}' returned {} result(s)", searchTerm, results.size());
    return results;
}
