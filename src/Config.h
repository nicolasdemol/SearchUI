#pragma once

#include <yaml-cpp/yaml.h>
#include <spdlog/spdlog.h>

#include <atomic>
#include <latch>
#include <string>

namespace Sample {
    class Debug {
    public:
        [[nodiscard]] inline spdlog::level::level_enum GetLogLevel() const noexcept { return _logLevel; }
        [[nodiscard]] inline spdlog::level::level_enum GetFlushLevel() const noexcept { return _flushLevel; }

        void LoadFromYAML(const YAML::Node& node);

    private:
        spdlog::level::level_enum _logLevel{spdlog::level::trace};
        spdlog::level::level_enum _flushLevel{spdlog::level::trace};
    };

    class Config {
    public:
        [[nodiscard]] inline const Debug& GetDebug() const noexcept { return _debug; }
        [[nodiscard]] static const Config& GetSingleton() noexcept;

    private:
        void LoadFromFile();
        Debug _debug;
    };
}  // namespace Sample
