"""Inspect SKSE metadata without executing the DLL (Python standard library only)."""
import pathlib
import struct
import sys


def verify(path):
    data = pathlib.Path(path).read_bytes()

    def u16(offset):
        return struct.unpack_from("<H", data, offset)[0]

    def u32(offset):
        return struct.unpack_from("<I", data, offset)[0]

    def require(condition, message):
        if not condition:
            raise ValueError(message)

    require(data[:2] == b"MZ", "Not a PE file")
    pe = u32(0x3C)
    require(data[pe:pe + 4] == b"PE\0\0", "Invalid PE signature")
    require(u16(pe + 4) == 0x8664, "Plugin is not AMD64")
    optional = pe + 24
    require(u16(optional) == 0x20B, "Plugin is not PE32+")
    section_table = optional + u16(pe + 20)
    sections = []
    for index in range(u16(pe + 6)):
        section = section_table + index * 40
        sections.append((u32(section + 12), u32(section + 16), u32(section + 20)))

    def file_offset(rva):
        for start, size, raw in sections:
            if start <= rva < start + size:
                return raw + rva - start
        raise ValueError(f"RVA outside file-backed PE sections: {rva:#x}")

    def cstring(offset):
        return data[offset:data.index(b"\0", offset)].decode("ascii")

    exports = file_offset(u32(optional + 112))
    functions = file_offset(u32(exports + 28))
    names = file_offset(u32(exports + 32))
    ordinals = file_offset(u32(exports + 36))
    symbols = {}
    for index in range(u32(exports + 24)):
        name = cstring(file_offset(u32(names + index * 4)))
        symbols[name] = file_offset(u32(functions + u16(ordinals + index * 2) * 4))
    for name in ("SKSEPlugin_Load", "SKSEPlugin_Query", "SKSEPlugin_Version"):
        require(name in symbols, f"Missing export: {name}")

    # SKSEPluginVersionData layout from SKSE 2.3.1 PluginAPI.h, not a source-text check.
    version = symbols["SKSEPlugin_Version"]
    require(u32(version) == 1, "Unexpected SKSE metadata schema")
    require(cstring(version + 8) == "SearchUI", "Incorrect plugin name")
    flags_ex = u32(version + 0x304)
    flags = u32(version + 0x308)
    require(flags_ex == 3, "Missing NG structure independence / Address Library v5 flag")
    require(flags == 1, "Expected Address Library version independence")
    # SKSE ignores compatibleVersions when AddressLibrary is set. CommonLib's
    # VersionNumber default constructor fills these unused slots with 1.0.0.
    require(u32(version + 0x34C) == 0, "Global minimum SKSE version would exclude legacy builds")
    print(f"PASS: {path}: AMD64; Load/Query/Version exports; versionIndependenceEx={flags_ex}; "
          f"versionIndependence={flags}; runtime whitelist bypassed; minimumSKSE=0")


if __name__ == "__main__":
    if len(sys.argv) != 2:
        sys.exit("Usage: python test/verify_plugin.py <SearchUI.dll>")
    verify(sys.argv[1])
