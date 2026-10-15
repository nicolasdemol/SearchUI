# Third-party notices

SearchUI's combined plugin is distributed under GPL-3.0-or-later with the
additional permissions described in README.md and EXCEPTIONS.md. The notices
below preserve the original terms of components used by the project. Include
this file, LICENSE, EXCEPTIONS.md and the entire licenses directory in releases.

## CommonLibSSE-NG

- Project: https://github.com/alandtse/CommonLibSSE-NG
- Version: 8.1.0, commit `3c0f5a87c3b166c9a6712d5c3bd180e9ac5ad0fd`.
- License: GPL-3.0-or-later with Modding Exception and GPL-3.0 Linking Exception
  (with Corresponding Source).
- Verbatim upstream texts: `licenses/CommonLibSSE-NG/COPYING.txt` and
  `licenses/CommonLibSSE-NG/EXCEPTIONS.md`.
- Historical MIT notices: `licenses/CommonLibSSE-NG/LICENSE-MIT.txt`.
- The instruction decoder hde64 is obtained from MinHook v1.3.4:
  https://github.com/TsudaKageyu/minhook/tree/v1.3.4/src/hde . Its original notice
  is preserved in `licenses/CommonLibSSE-NG/LICENSE-hde64.txt`.
- OpenVR submodule: https://github.com/ValveSoftware/openvr at commit
  `60eb187801956ad277f1cae6680e3a410ee0873b`; BSD-3-Clause, `licenses/openvr.txt`.

## Original project template

SearchUI contains code and build/distribution scaffolding derived from the
CommonLibSSE sample plugin template. Copyright 2022 Color-Glass Studios, LLC.
The original Apache License 2.0, including that attribution, is preserved without
alteration in `licenses/Apache-2.0-legacy.txt`. Changes made for SearchUI are
recorded in this repository's Git history. The GPL distribution of the combined
work does not erase the original Apache terms or copyright notices.

## Native dependencies

The following versions are those installed for the SearchUI 1.2.0 build. The
copyright files are copied verbatim from that build's vcpkg installation and
include the copyright holders and complete applicable license terms.

| Component | Version | License text | Upstream source |
| --- | --- | --- | --- |
| DirectXMath | 2025-04-03 | `licenses/directxmath.txt` (MIT) | https://github.com/microsoft/DirectXMath |
| DirectXTK | 2025-10-27 | `licenses/directxtk.txt` (MIT) | https://github.com/microsoft/DirectXTK |
| fmt | 12.1.0 | `licenses/fmt.txt` (MIT) | https://github.com/fmtlib/fmt |
| rapidcsv | 8.90 | `licenses/rapidcsv.txt` (BSD-3-Clause) | https://github.com/d99kris/rapidcsv |
| spdlog | 1.16.0 | `licenses/spdlog.txt` (MIT and bundled notices) | https://github.com/gabime/spdlog |
| yaml-cpp | 0.8.0, vcpkg port revision 3 | `licenses/yaml-cpp.txt` (MIT) | https://github.com/jbeder/yaml-cpp |

Dependency selection and build patches are recorded in `vcpkg.json`,
`vcpkg-configuration.json`, and `cmake/CommonLib.cmake`. When updating
dependencies, refresh the notices and accompanying dependency sources as well.

## Separately installed runtime dependencies

Skyrim/Creation Kit (Bethesda), SKSE, SkyUI, UIExtensions, and Address Library
are required or used for development but are not relicensed by SearchUI.
Obtain them from their respective publishers. Do not interpret SearchUI's open
permissions as permission to redistribute another author's assets or the game.
