# Source and release provenance

Repository: https://github.com/nicolasdemol/SearchUI

SearchUI 1.2.0's native and Papyrus implementation is recorded at commit
`62da4e3` (Release SearchUI 1.2.0 with persistent settings and safer searches).
The licensing correction changes license documents and the DLL's license
metadata, not search behavior. A license-corrected build must identify the full
new commit ID and its SHA-256 hashes in its accompanying release manifest.

## Verified 1.2 release and licensing correction

On 15 October 2026, the SHA-256 in Nexus file 807816's public VirusTotal link
matched the retained original `SearchUI.zip`:
`d2d0dd8529b54f74fb1bcbf544d09f7df250b48d9fca1efa90588a42a07d7286`.
Its DLL matches the retained final 1.2.0 package (SHA-256
`0d15af557a42589510757b37db1e50f5e5d5ce0ece7e886dfd4d5f815662f345`).

The licensing correction is tagged `v1.2.0-license-fix`, commit
`66d3a955b219fc88f84c76692917988c1353e5fb`. Its DLL SHA-256 is
`2c44ac55f8acf990efc55b9582e356eeebda4d8cd0f942e27e731e5c6b2799f4`.
The executable `.text` section is byte-identical to the original Nexus DLL;
the relink changes `.rdata` and `.rsrc`. The ESP, compiled Papyrus scripts and
Papyrus sources are byte-identical to those in the original Nexus archive.

The corrected binary and its source/dependency bundle are distributed together
on the [Nexus Files page](https://www.nexusmods.com/skyrimspecialedition/mods/155713?tab=files):

| Download | Nexus file ID | SHA-256 |
| --- | --- | --- |
| SearchUI - License correction | 820734 | `6f9bc0fea6b1cdd80b78746f04c787f38c2b80c875ab4c1b27d782b7f2f4804e` |
| SearchUI 1.2.0 - Source code and dependencies | 820733 | `8ab107fa5a04f827dfe651b97d65a57488161979ecb3129963a3174d57a5d6e0` |

Both archives contain `RELEASE-MANIFEST.json` with the source commit and file
hashes. The source bundle also contains actual dependency source trees and
vcpkg SPDX provenance records. These release archives remain tied to the tag;
later documentation-only updates do not change the release source revision.

Do not assign the current source to historical versions 1.1.4 or earlier without
checking their original build inputs.

The public repository contains the C++ sources, headers, production Papyrus
sources, build configuration, dependency baselines and build instructions in
README.md. CommonLibSSE-NG is pinned to
`3c0f5a87c3b166c9a6712d5c3bd180e9ac5ad0fd` (8.1.0).

For binary distribution, include the matching SearchUI source snapshot and
dependency sources (including CommonLibSSE-NG and its OpenVR/hde64 dependencies)
in a source archive available from the same download location. Preserve the
build patches and dependency notices. Do not substitute a source archive from
an unrelated or newer build.

Rebuild instructions are in README.md. The existing SearchUI.esp is a Creation
Kit plugin asset and is not produced by the native build. Papyrus compilation
requires the separately installed Creation Kit compiler and the imported
Skyrim, SKSE, SkyUI and UIExtensions script interfaces.
