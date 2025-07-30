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

        static std::vector<Result> FindFormsByName(const std::string& searchTerm, bool exactMatch = false);
    };
}  // namespace Sample
