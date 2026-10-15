# Source and release provenance

Repository: https://github.com/nicolasdemol/SearchUI

SearchUI 1.2.0's native and Papyrus implementation is recorded at commit
`62da4e3` (Release SearchUI 1.2.0 with persistent settings and safer searches).
The licensing correction changes license documents and the DLL's license
metadata, not search behavior. A license-corrected build must identify the full
new commit ID and its SHA-256 hashes in its accompanying release manifest.

The original Nexus 1.2 download must be compared with the retained local release
before claiming a byte-for-byte match. Version labels alone do not establish
that match. Do not assign the current source to historical versions 1.1.4 or
earlier without checking their original build inputs.

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
