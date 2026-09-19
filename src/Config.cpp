#include "Config.h"

#include <yaml-cpp/yaml.h>

#include <fstream>
#include <mutex>

using namespace SearchUI;

void Debug::LoadFromYAML(const YAML::Node& node) {
    if (node["logLevel"]) {
        try {
            _logLevel = spdlog::level::from_str(node["logLevel"].as<std::string>());
        } catch (const std::exception& e) {
            spdlog::warn("Invalid logLevel in config: {}", e.what());
        }
    }
    if (node["flushLevel"]) {
        try {
            _flushLevel = spdlog::level::from_str(node["flushLevel"].as<std::string>());
        } catch (const std::exception& e) {
            spdlog::warn("Invalid flushLevel in config: {}", e.what());
        }
    }
}

const Config& Config::GetSingleton() noexcept {
    static Config instance;
    static std::atomic_bool initialized{false};
    static std::latch latch(1);

    if (!initialized.exchange(true)) {
        instance.LoadFromFile();
        latch.count_down();
    }

    latch.wait();
    return instance;
}

void Config::LoadFromFile() {
    try {
        if (!std::filesystem::exists("Data/SKSE/Plugins/SearchUI.yaml")) return;
        YAML::Node root = YAML::LoadFile("Data/SKSE/Plugins/SearchUI.yaml");
        if (root["debug"]) {
            _debug.LoadFromYAML(root["debug"]);
        } else {
            spdlog::warn("Missing 'debug' section in config.");
        }
    } catch (const std::exception& e) {
        spdlog::warn("Failed to load config: {}", e.what());
    }
}
