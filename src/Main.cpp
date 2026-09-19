#include "Config.h"
#include "Papyrus.h"

#include <stddef.h>
#include <SearchUI/Search.h>
#include <SearchUI/AsyncSearch.h>

using namespace RE;
using namespace RE::BSScript;
using namespace SearchUI;
using namespace SKSE;
using namespace SKSE::log;
using namespace SKSE::stl;

namespace {
    void InitializeLogging() {
        auto path = log_directory();
        if (!path) {
            report_and_fail("Unable to lookup SKSE logs directory.");
        }
        *path /= PluginDeclaration::GetSingleton()->GetName();
        *path += L".log";

        std::shared_ptr<spdlog::logger> log;
        if (IsDebuggerPresent()) {
            log = std::make_shared<spdlog::logger>(
                "Global", std::make_shared<spdlog::sinks::msvc_sink_mt>());
        } else {
            log = std::make_shared<spdlog::logger>(
                "Global", std::make_shared<spdlog::sinks::basic_file_sink_mt>(path->string(), true));
        }
        spdlog::set_default_logger(std::move(log));
        spdlog::set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%n] [%l] [%t] [%s:%#] %v");
        const auto& debug = SearchUI::Config::GetSingleton().GetDebug();
        spdlog::set_level(debug.GetLogLevel());
        spdlog::flush_on(debug.GetFlushLevel());
    }

    void InitializePapyrus() {
        log::trace("Initializing Papyrus binding...");
        const auto* papyrus = GetPapyrusInterface();
        if (papyrus && papyrus->Register(SearchUI::RegisterPapyrusFuncs)) {
            log::debug("Papyrus functions bound.");
        } else {
            stl::report_and_fail("Failure to register Papyrus bindings.");
        }
    }

    void InitializeMessaging() {
        const auto* messaging = GetMessagingInterface();
        if (!messaging || !messaging->RegisterListener([](MessagingInterface::Message* message) {
            log::debug("SKSE lifecycle message {}", message->type);
            switch (message->type) {
                // Skyrim lifecycle events.
                case MessagingInterface::kPostLoad: // Called after all plugins have finished running SKSEPlugin_Load.
                    // It is now safe to do multithreaded operations, or operations against other plugins.
                case MessagingInterface::kPostPostLoad: // Called after all kPostLoad message handlers have run.
                case MessagingInterface::kInputLoaded: // Called when all game data has been found.
                    break;
                case MessagingInterface::kDataLoaded: // All ESM/ESL/ESP plugins have loaded, main menu is now active.
                    // It is now safe to access form data.
                    logger::info("DataLoaded: scheduling immutable search index construction.");
                    SKSE::GetTaskInterface()->AddTask([] { Search::BuildIndex(); });
                    break;

                // Skyrim game events.
                case MessagingInterface::kNewGame: // Player starts a new game from main menu.
                case MessagingInterface::kPreLoadGame: // Player selected a game to load, but it hasn't loaded yet.
                    // Data will be the name of the loaded save.
                    AsyncSearch::Get().Cancel();
                    break;
                case MessagingInterface::kPostLoadGame: // Player's selected save game has finished loading.
                    // Data will be a boolean indicating whether the load was successful.
                case MessagingInterface::kSaveGame: // The player has saved a game.
                    // Data will be the save name.
                case MessagingInterface::kDeleteGame: // The player deleted a saved game from within the load menu.
                    break;
            }
        })) {
            stl::report_and_fail("Unable to register message listener.");
        }
    }
}

SKSEPluginLoad(const LoadInterface* skse) {
    try {
        InitializeLogging();

        auto* plugin = PluginDeclaration::GetSingleton();
        auto version = plugin->GetVersion();
        log::info("{} {} is loading...", plugin->GetName(), version);

        log::info("Runtime {}, SKSE {:#010x}; CommonLibSSE-NG 8.1.0; Address Library v5 enabled.",
                  skse->RuntimeVersion().string("."), skse->SKSEVersion());
        Init(skse, SKSE::InitInfo{.log = false});
        log::info("SKSE API initialization complete.");
        if (!GetTaskInterface()) {
            log::critical("SKSE task interface unavailable.");
            return false;
        }
        InitializeMessaging();
        InitializePapyrus();

        log::info("{} has finished loading.", plugin->GetName());
        return true;
    } catch (const std::exception& e) {
        log::critical("SearchUI initialization failed: {}", e.what());
        return false;
    }
}
