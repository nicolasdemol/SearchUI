Scriptname SearchUIController extends Quest

import SearchAPI

import UIExtensions

ObjectReference Property SearchContainer Auto
int Property SearchHotkey = 62 Auto ; F4
int Property MaxResults = 1000 Auto
int Property ConsumableQuantity = 50 Auto

bool Property EnableWeapons = true Auto
bool Property EnableArmors = true Auto
bool Property EnableBooks = false Auto
bool Property EnablePotions = true Auto
bool Property EnableMiscItems = false Auto
bool Property EnableIngredients = true Auto
bool Property EnableAmmo = true Auto
bool Property EnableLights = false Auto


int Property EnchantmentMode = 0 Auto ; 0 all, 1 not enchanted, 2 enchanted
bool busy = false
int sessionEpoch = 0

Function Initialize()
    sessionEpoch += 1
    busy = false
    SearchAPI.CancelSearch()
    RestoreSettings()
EndFunction

; The external settings file, never the loaded save's properties, is authoritative.
; Fixed fallbacks also prevent stale save values returning if no file exists yet.
Function RestoreSettings()
    SearchAPI.LoadPreferences()
    SearchHotkey = SearchAPI.GetPreference("hotkey", 62)
    MaxResults = SearchAPI.GetPreference("maxResults", 1000)
    ConsumableQuantity = SearchAPI.GetPreference("quantity", 50)
    int mask = SearchAPI.GetPreference("categories", 107)
    EnableWeapons = Math.LogicalAnd(mask, 1) != 0
    EnableArmors = Math.LogicalAnd(mask, 2) != 0
    EnableBooks = Math.LogicalAnd(mask, 4) != 0
    EnablePotions = Math.LogicalAnd(mask, 8) != 0
    EnableMiscItems = Math.LogicalAnd(mask, 16) != 0
    EnableIngredients = Math.LogicalAnd(mask, 32) != 0
    EnableAmmo = Math.LogicalAnd(mask, 64) != 0
    EnableLights = Math.LogicalAnd(mask, 128) != 0
    EnchantmentMode = SearchAPI.GetPreference("enchantment", 0)
    UnregisterForAllKeys()
    RegisterForKey(SearchHotkey)
EndFunction

Function SaveSettings()
    if !SearchAPI.SaveSettings(SearchHotkey, MaxResults, ConsumableQuantity, GetCategoryMask(), EnchantmentMode)
        Debug.Notification("SearchUI: could not save settings. See SearchUI.log.")
    endif
EndFunction

Event OnInit()
    Initialize()
EndEvent

Function SetHotkey(int newKey)
	if newKey != SearchHotkey
		UnregisterForKey(SearchHotkey)
		SearchHotkey = newKey
		RegisterForKey(SearchHotkey)
	endif
EndFunction

Function SetMaxResults(int newValue)
	MaxResults = newValue
EndFunction

Function SetConsumableQuantity(int newQuantity)
    ConsumableQuantity = newQuantity
EndFunction

Event OnKeyDown(Int keyCode)
    if keyCode != SearchHotkey || busy || Utility.IsInMenuMode()
        return
    endif
    if !UIExtensions.GetMenu("UITextEntryMenu")
        Debug.Notification("SearchUI: UIExtensions is missing or not ready.")
        return
    endif
    busy = true
    int epoch = sessionEpoch
    int result = UIExtensions.OpenMenu("UITextEntryMenu")
    if result == 1 && epoch == sessionEpoch
        string term = UIExtensions.GetMenuResultString("UITextEntryMenu")
        if term != ""
            ProcessSearch(term)
        endif
    endif
    if epoch == sessionEpoch
        busy = false
    endif
EndEvent
int Function GetCategoryMask()
	int mask = 0
	if EnableWeapons
		mask = mask + 1
	endif
	if EnableArmors
		mask = mask + 2
	endif
	if EnableBooks
		mask = mask + 4
	endif
	if EnablePotions
		mask = mask + 8
	endif
	if EnableMiscItems
		mask = mask + 16
	endif
	if EnableIngredients
		mask = mask + 32
	endif
	if EnableAmmo
		mask = mask + 64
	endif
	if EnableLights
		mask = mask + 128
	endif
	return mask
EndFunction


Function ProcessSearch(string term)
    if !SearchContainer
        Debug.Notification("SearchUI: search container is not set.")
        return
    endif
    if term == ""
        return
    endif
    if GetCategoryMask() == 0
        Debug.Notification("SearchUI: enable at least one search category in MCM.")
        return
    endif
    int epoch = sessionEpoch
    SearchAPI.StartItemSearch(term, GetCategoryMask(), EnchantmentMode)
    int maxWait = 300
    while !SearchAPI.IsSearchFinished() && maxWait > 0 && epoch == sessionEpoch
        Utility.WaitMenuMode(0.1)
        maxWait -= 1
    endwhile
    if epoch != sessionEpoch
        return
    endif
    if !SearchAPI.IsSearchFinished()
        SearchAPI.CancelSearch()
        Debug.Notification("SearchUI: search timed out. Please try again.")
        return
    endif
    if SearchAPI.DidSearchFail()
        Debug.Notification("SearchUI: search unavailable. See SearchUI.log.")
        return
    endif
    int count = SearchAPI.GetSearchCount()
    if count > MaxResults
        Debug.Notification("Too many results (" + count + "). Refine the name or search categories.")
        return
    endif
    if count == 0
        Debug.Notification("No results found.")
        return
    endif
    SearchContainer.RemoveAllItems()
    SearchAPI.AddResultsToContainer(SearchContainer, MaxResults, ConsumableQuantity)
    if epoch == sessionEpoch
        SearchContainer.Activate(Game.GetPlayer())
    endif
EndFunction
