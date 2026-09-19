#include <iostream>
#include <stdexcept>

namespace {
    void Require(bool condition, const char* message) {
        if (!condition) throw std::runtime_error(message);
    }

    void CheckIDs(bool checkSegments) {
        // These are the same relocation pairs used by CommonLib's SearchUI APIs.
        const std::array<std::pair<const char*, REL::RelocationID>, 4> globals{{
            {"TESDataHandler", REL::RelocationID{514141, 400269}},
            {"UI", REL::RelocationID{514178, 400327}},
            {"TESForm::allForms", REL::RelocationID{514351, 400507}},
            {"TESForm::allFormsMapLock", REL::RelocationID{514360, 400517}},
        }};
        for (const auto& [name, id] : globals) {
            const auto offset = id.offset();
            Require(offset != 0, "Address Library returned a null RVA");
            if (checkSegments) {
                const auto& data = REL::Module::get().segment(REL::Segment::data);
                Require(offset >= data.offset() && offset + sizeof(void*) <= data.offset() + data.size(),
                        "Game global is outside the executable's data section");
            }
            std::cout << name << " RVA=0x" << std::hex << offset << std::dec << '\n';
        }
    }
}

// Maps the executable at rest through CommonLib's testing API. Does not start
// Skyrim, call game functions, install hooks, or substitute for an in-game test.
int wmain(int argc, wchar_t** argv) {
    try {
        Require(argc == 3, "Usage: SearchUIRuntimeProbe <SkyrimSE.exe> <Address Library directory>");
        const std::filesystem::path libraries(argv[2]);
        Require(REL::Module::inject(argv[1]), "Cannot map Skyrim executable");
        const auto version = REL::Module::get().version();
        Require(version == REL::Version(1, 7, 104, 0), "Expected real Skyrim 1.7.104 executable");
        Require(REL::Module::IsAE() && !REL::Module::IsSE(), "1.7.104 was not classified as AE");
        Require(REL::IDDB::inject((libraries / L"versionlib-1-7-104-0.bin").wstring(), version),
                "Cannot load actual 1.7.104 Address Library (auto-detection)");
        std::cout << "Real executable " << version.string(".") << ": AE; format 5 loaded\n";
        CheckIDs(true);

        // Older executables are unavailable: exercise classification and the actual
        // installed databases with CommonLib's synthetic module, explicitly.
        for (const auto legacy : {REL::Version(1, 6, 1170, 0), REL::Version(1, 5, 97, 0)}) {
            Require(REL::Module::mock(legacy), "Cannot create legacy test module");
            Require(REL::Module::IsAE() == (legacy.minor() >= 6), "Incorrect legacy runtime classification");
            const auto filename = (legacy.minor() >= 6 ? L"versionlib-" : L"version-") +
                                  legacy.wstring() + L".bin";
            Require(REL::IDDB::inject((libraries / filename).wstring(), legacy), "Cannot load legacy Address Library");
            std::cout << "Synthetic module " << legacy.string(".") << ": actual legacy library loaded\n";
            CheckIDs(false);
        }
        std::cout << "PASS: offline runtime classification, format 1/2/5 parsing, and SearchUI global relocations\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << "FAIL: " << error.what() << '\n';
        return 1;
    }
}
