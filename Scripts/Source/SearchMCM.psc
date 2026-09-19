Scriptname SearchMCM extends SKI_ConfigBase

SearchUIController Property Controller Auto

int Property searchKey = 62 Auto ; F4 par défaut
int Property maxResults = 200 Auto
int Property consumableQuantity = 50 Auto


int optionSearchKey
int optionMaxResults
int optionConsumableQuantity
int optionEnchantment

int optionWeapons
int optionArmors
int optionBooks
int optionPotions
int optionMiscItems
int optionIngredients
int optionAmmo
int optionLights


Event OnInit()
	parent.OnInit()
	ModName = "SearchUI"
EndEvent

Event OnConfigInit()
    if Controller
        Controller.Initialize()
    endif
EndEvent

Event OnGameReload()
    parent.OnGameReload()
    if Controller
        Controller.Initialize()
    endif
EndEvent

Event OnConfigOpen()
    if Controller
        Controller.RestoreSettings()
    endif
EndEvent

; Closing a menu must never export stale properties from an older save.
; Each accepted edit is persisted in its own callback below.

Event OnPageReset(string page)
	if !Controller
		return
	endif
	Controller.RestoreSettings()
	SetCursorFillMode(TOP_TO_BOTTOM)

	AddHeaderOption("Configuration")
	optionSearchKey = AddKeyMapOption("Search Key", Controller.SearchHotkey)
	optionMaxResults = AddSliderOption("Max Results", Controller.MaxResults, "{0}")
	optionConsumableQuantity = AddSliderOption("Consumable Quantity", Controller.ConsumableQuantity, "{0}")
    optionEnchantment = AddMenuOption("Enchanted items", EnchantmentLabel(Controller.EnchantmentMode))
    AddEmptyOption()

	SetCursorPosition(1)

	AddHeaderOption("Search Categories")
	optionWeapons       = AddToggleOption("Weapons", Controller.EnableWeapons)
	optionArmors        = AddToggleOption("Armors", Controller.EnableArmors)
	optionBooks         = AddToggleOption("Books", Controller.EnableBooks)
	optionPotions       = AddToggleOption("Potions", Controller.EnablePotions)
	optionMiscItems     = AddToggleOption("Misc Items", Controller.EnableMiscItems)
	optionIngredients   = AddToggleOption("Ingredients", Controller.EnableIngredients)
	optionAmmo          = AddToggleOption("Ammo", Controller.EnableAmmo)
	optionLights        = AddToggleOption("Lights", Controller.EnableLights)
EndEvent

Event onOptionSelect(int option)
	if option == optionWeapons
		Controller.EnableWeapons = !Controller.EnableWeapons
		SetToggleOptionValue(optionWeapons, Controller.EnableWeapons)
	elseif option == optionArmors
		Controller.EnableArmors = !Controller.EnableArmors
		SetToggleOptionValue(optionArmors, Controller.EnableArmors)
	elseif option == optionBooks
		Controller.EnableBooks = !Controller.EnableBooks
		SetToggleOptionValue(optionBooks, Controller.EnableBooks)
	elseif option == optionPotions
		Controller.EnablePotions = !Controller.EnablePotions
		SetToggleOptionValue(optionPotions, Controller.EnablePotions)
	elseif option == optionMiscItems
		Controller.EnableMiscItems = !Controller.EnableMiscItems
		SetToggleOptionValue(optionMiscItems, Controller.EnableMiscItems)
	elseif option == optionIngredients
		Controller.EnableIngredients = !Controller.EnableIngredients
		SetToggleOptionValue(optionIngredients, Controller.EnableIngredients)
	elseif option == optionAmmo
		Controller.EnableAmmo = !Controller.EnableAmmo
		SetToggleOptionValue(optionAmmo, Controller.EnableAmmo)
	elseif option == optionLights
		Controller.EnableLights = !Controller.EnableLights
		SetToggleOptionValue(optionLights, Controller.EnableLights)
	else
		return
	endif
	Controller.SaveSettings()
EndEvent

Event OnOptionKeyMapChange(int option, int keyCode, string conflictControl, string conflictName)
	if option == optionSearchKey
		searchKey = keyCode
		Controller.SetHotkey(searchKey)
		SetKeymapOptionValue(optionSearchKey, keyCode)
		Controller.SaveSettings()
	endif
EndEvent

Event OnOptionSliderOpen(int option)
	if option == optionMaxResults
		SetSliderDialogStartValue(Controller.MaxResults)
		SetSliderDialogDefaultValue(200)
		SetSliderDialogRange(10, 5000)
		SetSliderDialogInterval(10)
	elseif option == optionConsumableQuantity
        SetSliderDialogStartValue(Controller.ConsumableQuantity)
        SetSliderDialogDefaultValue(50)
        SetSliderDialogRange(1, 500)
        SetSliderDialogInterval(1)
    endif
EndEvent

Event OnOptionSliderAccept(int option, float value)
	if option == optionMaxResults
		maxResults = value as int
		Controller.SetMaxResults(maxResults)
		SetSliderOptionValue(optionMaxResults, maxResults, "{0}")
		Controller.SaveSettings()
	elseif option == optionConsumableQuantity
        consumableQuantity = value as int
        Controller.SetConsumableQuantity(consumableQuantity)
        SetSliderOptionValue(optionConsumableQuantity, consumableQuantity, "{0}")
        Controller.SaveSettings()
    endif
EndEvent
string Function EnchantmentLabel(int mode)
    if mode == 1
        return "Not enchanted"
    elseif mode == 2
        return "Enchanted only"
    endif
    return "All"
EndFunction

Event OnOptionMenuOpen(int option)
    if option == optionEnchantment
        string[] choices = new string[3]
        choices[0] = "All"
        choices[1] = "Not enchanted"
        choices[2] = "Enchanted only"
        SetMenuDialogOptions(choices)
        SetMenuDialogStartIndex(Controller.EnchantmentMode)
        SetMenuDialogDefaultIndex(0)
    endif
EndEvent

Event OnOptionMenuAccept(int option, int value)
    if option == optionEnchantment && value >= 0 && value <= 2
        Controller.EnchantmentMode = value
        SetMenuOptionValue(optionEnchantment, EnchantmentLabel(value))
        Controller.SaveSettings()
    endif
EndEvent

Event OnOptionHighlight(int option)
    if option == optionEnchantment
        SetInfoText("Filter built-in item enchantments. This does not inspect enchantments applied by the player to inventory instances.")
    endif
EndEvent
