Scriptname SearchAPI

; Legacy exactMatch arguments are ignored: smart matching is always enabled.
Function RunSearch(string term, bool exactMatch, int categoryMask) Global native
int Function GetSearchCount() Global native
string Function GetSearchResultName(int index) Global native
Form Function GetSearchResult(int index) Global native
Function AddResultsToContainer(ObjectReference container, int maxResults, int consumableQuantity) Global native

Function StartAsyncSearch(string term, bool exactMatch, int categoryMask) Global native
bool Function IsSearchFinished() Global native

; Current API: smart matching, category and enchantment filters only.
Function StartItemSearch(string term, int categoryMask, int enchantmentMode) Global native
bool Function SaveSettings(int hotkey, int maxResults, int quantity, int categoryMask, int enchantmentMode) Global native

; Compatibility with previous scripts: pluginFilter/exactMatch/smart ignored;
; plugin getters return an empty string. New code should use the API above.
Function StartFilteredSearch(string term, bool exactMatch, int categoryMask, string pluginFilter, int enchantmentMode) Global native
Function CancelSearch() Global native
bool Function DidSearchFail() Global native
string Function GetSearchResultPlugin(int index) Global native
bool Function LoadPreferences() Global native
int Function GetPreference(string key, int fallback) Global native
string Function GetPluginFilter() Global native
bool Function SavePreferences(int hotkey, bool smart, int maxResults, int quantity, int categoryMask, string pluginFilter, int enchantmentMode) Global native
