# SearchUI

**Find Skyrim items by name and take what you need from an in-game search chest.**

SearchUI combines a native SKSE search engine with UIExtensions text input and a
SkyUI configuration menu. Search vanilla and mod-added items without looking up
FormIDs or entering console commands.

[Download on Nexus Mods](https://www.nexusmods.com/skyrimspecialedition/mods/155713)
· [Changelog](CHANGELOG.md)
· [Report an issue](https://github.com/nicolasdemol/SearchUI/issues)

## Features

- **Smart matching:** type partial words in any order. `ebo kni` can match
  `Ebony Knight`, and every word must appear in the item's name.
- **Enchantment filter:** choose All, Not enchanted, or Enchanted only.
- **Search categories:** weapons, armor, books, potions, miscellaneous items,
  ingredients, ammunition, and lights.
- **Persistent preferences:** changes are saved immediately and restored across
  saves and new games using the same settings file.
- **Unicode matching:** case-insensitive searches with Latin accent folding;
  `epee` can find `Épée`.
- **Asynchronous searches:** game data is copied into an index on the main thread;
  the search worker processes that index without accessing game objects.
- **Configurable controls:** change the hotkey, result limit, and stack quantities
  for ammunition, ingredients, and miscellaneous items in the MCM.

Smart Search is always enabled in 1.2.0. There is no plugin-name filter.

## Requirements

| Dependency | Purpose |
| --- | --- |
| [SKSE](https://skse.silverlock.org/) | Loads the native plugin; select the version for your Skyrim runtime. |
| [Address Library](https://www.nexusmods.com/skyrimspecialedition/mods/32444) | Provides runtime-specific addresses; install the matching edition/version. |
| [SkyUI](https://www.nexusmods.com/skyrimspecialedition/mods/12604) | Provides the MCM and inventory interface. |
| [UIExtensions](https://www.nexusmods.com/skyrimspecialedition/mods/17561) | Provides the text-entry menu. |

VR requires the corresponding VR dependencies and a VR-compatible UIExtensions
text-entry menu. See the Nexus page for the VR requirements.

## Runtime compatibility

SearchUI uses one CommonLibSSE-NG DLL with SE, AE, and VR support enabled.
It is not restricted to Skyrim 1.7.104.

| Runtime | Validation for the 1.2 development build |
| --- | --- |
| Steam 1.7.104 / SKSE 2.3.1 | In-game loading, indexing, and searches observed; Address Library v5 checked. |
| 1.6.1170 | Support retained; offline Address Library checks passed. Not re-tested in-game. |
| 1.5.97 | Support retained; offline Address Library checks passed. Not re-tested in-game. |
| Other SE/AE runtimes and VR | Build targets retained; not individually verified for this release. |

The older-runtime probes use synthetic modules with real Address Libraries,
not running game executables. They do not prove full in-game compatibility.
The latest enchantment-setting fix has regression-test coverage; its final
save/load behavior still needs in-game confirmation.

## Installation and use

1. Install the requirements matching your game runtime.
2. Install the **complete SearchUI archive from Nexus** with your mod manager and
   enable `SearchUI.esp`. GitHub's source ZIP is not an installable mod.
3. Launch Skyrim through SKSE and allow the MCM to initialize.
4. Press **F4** (the default hotkey), enter a name or a few words, and confirm.
5. Take the items you want from the search chest.

Use the SearchUI MCM to select categories, filter enchantments, or adjust limits.
A search with no matches displays a notification. A blank search does not list
all items. If there are too many results, narrow the query or selected categories.

**The search chest is temporary.** A new successful search clears its contents;
do not use it to store items you want to keep.

When updating, close Skyrim and install the DLL and scripts from the same release
together. Check that older script overrides are not replacing the updated files.

## Settings and common questions

**Are settings tied to a character?**

No. Each accepted MCM edit is written to:

```text
Data/SKSE/Plugins/SearchUI.settings.yaml
```

The file is read when a game is initialized or loaded and when the MCM opens or
redraws. With MO2, it may be created in **Overwrite**. Keep it when updating;
profiles see whichever settings file their virtual filesystem exposes.

**What does the enchantment filter check?**

Enchantments built into the base item. It does not inspect enchantments applied
by the player to individual inventory items.

**What are lights?**

Skyrim's light-object category, mainly torches. Other mods may add named light
objects to the same category.

**Does Unicode matching provide an IME or clipboard support?**

No. Matching supports Unicode, but text entry still depends on UIExtensions and
the fonts/input facilities available in the game.

**Will dynamically renamed items appear under their new names?**

The index is built after game data loads. Later changes made by other mods are
not automatically reindexed.

## Troubleshooting

Check `SearchUI.log` in the SKSE log directory, normally under
`Documents/My Games/Skyrim Special Edition/SKSE`.

- Initialization logs report the runtime and native function registration.
- `Search index ready` confirms the item index was constructed.
- `Global preferences saved` and `Global preferences loaded` show the settings
  path and values, which helps diagnose unexpected MCM resets.

For a bug report, include your Skyrim runtime, SKSE and SearchUI versions, steps
to reproduce the problem, and the relevant log. Remove personal paths if needed.

Optional logging configuration: `Data/SKSE/Plugins/SearchUI.yaml`.

```yaml
debug:
  logLevel: info
  flushLevel: info
```

## Building from source

The project uses **C++23, CMake, Ninja, and vcpkg**. CommonLibSSE-NG **8.1.0** is
pinned to commit `3c0f5a87c3b166c9a6712d5c3bd180e9ac5ad0fd` in
[`cmake/CommonLib.cmake`](cmake/CommonLib.cmake). CMake fetches it and its OpenVR
submodule. Dependency registry baselines are also pinned.

### Native DLL

Install Visual Studio 2022 with the Desktop development with C++ workload and a
Windows SDK, CMake 3.21 or later, Ninja, Git, and vcpkg. Python 3 is optional for
the PE metadata check. In an **x64 Developer PowerShell**, adapt the vcpkg path:

```powershell
git clone https://github.com/nicolasdemol/SearchUI.git
cd SearchUI
$env:VCPKG_ROOT = 'C:/Tools/vcpkg'
$env:SkyrimPluginTargets = ''
cmake --preset build-release-msvc -DBUILD_TESTS=ON
cmake --build --preset release-msvc --parallel 6
ctest --test-dir build/release-msvc --output-on-failure
python test/verify_plugin.py build/release-msvc/SearchUI.dll
```

The output is `build/release-msvc/SearchUI.dll`. An empty `SkyrimPluginTargets`
prevents automatic deployment into a game/mod-manager directory during builds.

### Papyrus scripts

Use the Creation Kit Papyrus compiler with the Skyrim, SKSE, SkyUI, and
UIExtensions script sources. One import folder must contain
`TESV_Papyrus_Flags.flg`. Adapt these example paths:

```powershell
./tools/Build-Papyrus.ps1 -GamePath 'C:/Games/Skyrim Special Edition' -ImportPaths @(
    'C:/Sources/SKSE',
    'C:/Sources/Skyrim',
    'C:/Sources/SkyUI',
    'C:/Sources/UIExtensions'
)
```

This compiles the three production scripts from `Scripts/Source` to
`build/papyrus`. The script stops if compilation fails.

### Packaging

Combine the new DLL and compiled scripts with the existing `SearchUI.esp` from
the mod distribution. The ESP is not generated by the C++ build.

```text
SearchUI.esp
SKSE/Plugins/SearchUI.dll
Scripts/SearchAPI.pex
Scripts/SearchMCM.pex
Scripts/SearchUIController.pex
Scripts/Source/*.psc
```

Do not distribute your personal settings file. The legacy `contrib` template
scripts/ESP and root PPJ files are not the production SearchUI package.

### Tests and implementation notes

`SearchUITests` currently runs **59 checks** covering matching, Unicode,
enchantment/category filtering, cancellation, concurrent result publication,
settings persistence, schema migration, and case-insensitive Papyrus preference
keys. These tests do not launch Skyrim.

`test/verify_plugin.py` checks the DLL's x64 PE format and SKSE metadata. The
optional `BUILD_RUNTIME_PROBE=ON` target checks Address Libraries offline; see
[`test/RuntimeProbe.cpp`](test/RuntimeProbe.cpp) for its arguments and limitations.

The C++ namespace is `SearchUI`. Existing search natives are declared in
[`Scripts/Source/SearchAPI.psc`](Scripts/Source/SearchAPI.psc); compatibility shims
ignore the removed exact-match/plugin-filter settings.

Additional technical notes (French):
[1.2 implementation and validation](docs/community-1.2.0.md) ·
[Historical runtime migration audit](docs/runtime-1.7.104.md).

## Credits and license

Thanks to the SKSE, SkyUI, UIExtensions, CommonLibSSE-NG, and Address Library
contributors, and to everyone reporting issues and testing SearchUI.

Repository license: [Apache 2.0](LICENSE). Dependencies and distributed game assets
retain their respective licenses; the repository license does not replace those.
