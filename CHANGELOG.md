# Changelog

## 1.2.0

- Added an enchanted-item filter: All, Not enchanted, or Enchanted only.
- Save MCM edits immediately and restore global settings across saves and new games.
- Always enable Smart Search and remove its MCM toggle.
- Improve Unicode matching and fold Latin accents when searching.
- Build an immutable item index on the main thread for asynchronous searches.
- Handle empty results, failed searches, cancellations, and stale results explicitly.
- Prevent repeated UI openings while a search interaction is active.
- Fix preference-key case sensitivity that could reset Enchanted items to All.
- Update to pinned CommonLibSSE-NG 8.1.0 with Address Library v5 support, retaining
  the SE/AE/VR multi-runtime build. See the README for validation by runtime.
- Rename the C++ namespace and include directory from Sample to SearchUI.
- Add native regression tests, offline metadata/runtime checks, and a Papyrus build script.

The plugin-name search prototype is not included in this release. Settings from
that prototype are migrated without restoring the removed filter or Smart Search toggle.
