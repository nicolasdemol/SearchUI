#include <SearchUI/SearchCore.h>
#include <Windows.h>
#include <sstream>
#include <stdexcept>

namespace SearchUI {
    std::string NormalizeSearchTerm(const std::string& text) {
        if (text.empty()) return {};
        const auto length = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, text.data(), static_cast<int>(text.size()), nullptr, 0);
        if (!length) throw std::invalid_argument("Invalid UTF-8 search text");
        std::wstring wide(length, L'\0');
        MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, text.data(), static_cast<int>(text.size()), wide.data(), length);
        const auto decomposedLength = NormalizeString(NormalizationD, wide.data(), length, nullptr, 0);
        if (decomposedLength <= 0) throw std::runtime_error("Unicode normalization failed");
        std::wstring decomposed(decomposedLength, L'\0');
        const auto written = NormalizeString(NormalizationD, wide.data(), length, decomposed.data(), decomposedLength);
        if (written <= 0) throw std::runtime_error("Unicode normalization failed");
        decomposed.resize(written);
        // Only fold Latin accents; other scripts retain their combining marks.
        std::wstring folded;
        bool latin = false;
        for (wchar_t c : decomposed) {
            if (c >= 0x0300 && c <= 0x036F && latin) continue;
            folded += c;
            latin = (c >= L'A' && c <= L'Z') || (c >= L'a' && c <= L'z');
        }
        std::wstring lowered(folded.size(), L'\0');
        if (!LCMapStringEx(LOCALE_NAME_INVARIANT, LCMAP_LOWERCASE, folded.data(),
                static_cast<int>(folded.size()), lowered.data(), static_cast<int>(lowered.size()), nullptr, nullptr, 0))
            throw std::runtime_error("Unicode case folding failed");
        const auto bytes = WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, lowered.data(),
            static_cast<int>(lowered.size()), nullptr, 0, nullptr, nullptr);
        if (!bytes) throw std::runtime_error("UTF-8 encoding failed");
        std::string result(bytes, '\0');
        WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, lowered.data(), static_cast<int>(lowered.size()),
            result.data(), bytes, nullptr, nullptr);
        const auto first = result.find_first_not_of(" \t\n\r\f\v");
        if (first == std::string::npos) return {};
        return result.substr(first, result.find_last_not_of(" \t\n\r\f\v") - first + 1);
    }

    std::vector<SearchResult> FilterIndex(const SearchIndex& index, const std::string& term,
        std::uint32_t categories, int enchantment,
        const std::function<bool()>& cancelled) {
        const auto key = NormalizeSearchTerm(term);
        std::vector<SearchResult> results;
        if (key.empty()) return results;
        std::vector<std::string> words;
        std::istringstream tokens(key);
        for (std::string word; tokens >> word;) words.push_back(std::move(word));
        std::size_t visited = 0;
        for (const auto& item : index) {
            if ((visited++ % 64 == 0) && cancelled && cancelled()) return {};
            if (!(categories & item.category)) continue;
            if ((enchantment == 1 && item.enchanted) || (enchantment == 2 && !item.enchanted)) continue;
            const bool match = std::all_of(words.begin(), words.end(),
                [&](const auto& word) { return item.key.find(word) != std::string::npos; });
            if (match) results.push_back(item.result);
        }
        return results; // The index was sorted once at construction.
    }
}
