#include "config_parser/config_globals.h"
#include "config_parser/parse_config.h"
#include "config_parser/parse_keymap.h"
#include "key_action.h"
#include "keymap.h"
#include "led_display.h"
#include "rational.h"

static parser_error_t parseNoneAction(config_buffer_t *buffer, key_action_t *keyAction)
{
    keyAction->type = KeyActionType_None;
    return ParserError_Success;
}

static parser_error_t parseKeyStrokeAction(config_buffer_t *buffer, key_action_t *keyAction, uint8_t keyStrokeAction)
{
    uint8_t serialKeystrokeType = (SERIALIZED_KEYSTROKE_TYPE_MASK_KEYSTROKE_TYPE & keyStrokeAction) >> SERIALIZED_KEYSTROKE_TYPE_OFFSET_KEYSTROKE_TYPE;
    uint16_t scancode = keyStrokeAction & SERIALIZED_KEYSTROKE_TYPE_MASK_HAS_SCANCODE
        ? serialKeystrokeType == SerializedKeystrokeType_LongMedia ? ReadUInt16(buffer) : ReadUInt8(buffer)
        : 0;

    uint8_t modifiers = keyStrokeAction & SERIALIZED_KEYSTROKE_TYPE_MASK_HAS_MODIFIERS
        ? ReadUInt8(buffer)
        : 0;

    uint8_t secondaryRole = keyStrokeAction & SERIALIZED_KEYSTROKE_TYPE_MASK_HAS_LONGPRESS
        ? ReadUInt8(buffer) + 1
        : 0;
    
    keystroke_type_t keystrokeType;
    switch (serialKeystrokeType) {
        case SerializedKeystrokeType_Basic:
            keystrokeType = KeystrokeType_Basic;
            break;
        case SerializedKeystrokeType_ShortMedia:
        case SerializedKeystrokeType_LongMedia:
            keystrokeType = KeystrokeType_Media;
            break;
        case SerializedKeystrokeType_System:
            keystrokeType = KeystrokeType_System;
            break;
        default:
            return ParserError_InvalidSerializedKeystrokeType;
    }
    if (keyAction != NULL) {
        keyAction->type = KeyActionType_Keystroke;
        keyAction->keystroke.keystrokeType = keystrokeType;
        keyAction->keystroke.scancode = scancode;
        keyAction->keystroke.modifiers = modifiers;
        keyAction->keystroke.secondaryRole = secondaryRole;
    }
    return ParserError_Success;
}

static parser_error_t parseSwitchLayerAction(config_buffer_t *buffer, key_action_t *keyAction)
{
    uint8_t layer = ReadUInt8(buffer) + 1;
    switch_layer_mode_t mode = ReadUInt8(buffer);

    if (keyAction != NULL) {
        keyAction->type = KeyActionType_SwitchLayer;
        keyAction->switchLayer.layer = layer;
        keyAction->switchLayer.mode = mode;
    }
    return ParserError_Success;
}

static parser_error_t parseSwitchKeymapAction(config_buffer_t *buffer, key_action_t *keyAction, uint8_t keymapCount)
{
    uint8_t keymapIndex = ReadUInt8(buffer);

    if (keymapIndex >= keymapCount) {
        return ParserError_InvalidSerializedSwitchKeymapAction;
    }
    if (keyAction != NULL) {
        keyAction->type = KeyActionType_SwitchKeymap;
        keyAction->switchKeymap.keymapId = keymapIndex;
    }
    return ParserError_Success;
}

static parser_error_t parsePlayMacroAction(config_buffer_t *buffer, key_action_t *keyAction, uint8_t macroCount)
{
    uint8_t macroIndex = ReadUInt8(buffer);

    if (macroIndex >= macroCount) {
        return ParserError_InvalidSerializedPlayMacroAction;
    }
    if (keyAction != NULL) {
        keyAction->type = KeyActionType_PlayMacro;
        keyAction->playMacro.macroId = macroIndex;
    }
    return ParserError_Success;
}

static parser_error_t parseMouseAction(config_buffer_t *buffer, key_action_t *keyAction)
{
    keyAction->type = KeyActionType_Mouse;

    uint8_t mouseAction = ReadUInt8(buffer);
    if (mouseAction > SerializedMouseAction_Last) {
        return ParserError_InvalidSerializedMouseAction;
    }
    if (keyAction != NULL) {
        memset(&keyAction->mouseAction, 0, sizeof keyAction->mouseAction);
        keyAction->mouseAction = mouseAction;
    }

    return ParserError_Success;
}

static parser_error_t parseMouseMoveAction(config_buffer_t *buffer, key_action_t *keyAction) 
{
    navigation_mode_t navigationMode = ReadUInt8(buffer);
    if (navigationMode > NavigationMode_Media || navigationMode < NavigationMode_LayerDependant) {
        return ParserError_InvalidSerializedMouseMoveAction;
    }
    rational_t multiplier;

    multiplier.parts.top = ReadUInt8(buffer);
    multiplier.parts.bottom = ReadUInt8(buffer);

    if ((multiplier.parts.top == 0) ^ (multiplier.parts.bottom == 0)) {
        return ParserError_InvalidSerializedMouseMoveAction;
    }

    if (keyAction != NULL) {
        keyAction->type = KeyActionType_MouseMove;
        keyAction->mouseMoveAction.navigationMode = navigationMode;
        keyAction->mouseMoveAction.multiplier = multiplier;
   }

    return ParserError_Success;
}

static parser_error_t parseKeyAction(config_buffer_t *buffer, key_action_t *keyAction, uint8_t keymapCount, uint8_t macroCount)
{
    uint8_t keyActionType = ReadUInt8(buffer);

    switch (keyActionType) {
        case SerializedKeyActionType_None:
            return parseNoneAction( buffer, keyAction);
        case SerializedKeyActionType_KeyStroke ... SerializedKeyActionType_LastKeyStroke:
            return parseKeyStrokeAction(buffer, keyAction, keyActionType);
        case SerializedKeyActionType_SwitchLayer:
            return parseSwitchLayerAction(buffer, keyAction);
        case SerializedKeyActionType_SwitchKeymap:
            return parseSwitchKeymapAction(buffer, keyAction, keymapCount);
        case SerializedKeyActionType_Mouse:
            return parseMouseAction(buffer, keyAction);
        case SerializedKeyActionType_PlayMacro:
            return parsePlayMacroAction(buffer, keyAction, macroCount);
        case SerializedKeyActionType_MouseMove:
            return parseMouseMoveAction(buffer, keyAction);
    }
    return ParserError_InvalidSerializedKeyActionType;
}

static parser_error_t parseKeyActions(config_buffer_t *buffer, key_action_t keymap[MAX_KEY_COUNT_PER_MODULE], uint8_t keymapCount, uint8_t macroCount)
{
    parser_error_t errorCode;
    uint16_t actionCount = ReadCompactLength(buffer);

    if (actionCount > MAX_KEY_COUNT_PER_MODULE) {
        return ParserError_InvalidActionCount;
    }
    for (uint8_t actionIdx = 0; actionIdx < actionCount; actionIdx++) {
        errorCode = parseKeyAction(buffer, keymap == NULL ? NULL : &keymap[actionIdx],keymapCount, macroCount);
        if (errorCode != ParserError_Success) {
            return errorCode;
        }
    }
    return ParserError_Success;
}

static parser_error_t parseModule(config_buffer_t *buffer, key_action_t keymap[SLOT_COUNT][MAX_KEY_COUNT_PER_MODULE], uint8_t keymapCount, uint8_t macroCount)
{
    uint8_t moduleId = ReadUInt8(buffer);
    if (!IsModuleAttached(moduleId)) {
        keymap = NULL;
    }
    slot_t slotId = ModuleIdToSlotId(moduleId);
    if (slotId > SLOT_COUNT) {
        return ParserError_InvalidModuleCount;
    }
    return parseKeyActions(buffer, keymap == NULL ? NULL : keymap[slotId], keymapCount, macroCount);
}

static parser_error_t parseLayer(config_buffer_t *buffer, key_action_t keymap[SLOT_COUNT][MAX_KEY_COUNT_PER_MODULE], uint8_t keymapCount, uint8_t macroCount )
{
    parser_error_t errorCode;
    uint16_t moduleCount = ReadCompactLength(buffer);

    if (moduleCount > ModuleId_AllCount) {
        return ParserError_InvalidModuleCount;
    }
    for (uint8_t moduleIdx = 0; moduleIdx < moduleCount; moduleIdx++) {
        errorCode = parseModule(buffer, keymap, keymapCount, macroCount);
        if (errorCode != ParserError_Success) {
            return errorCode;
        }
    }
    return ParserError_Success;
}

parser_error_t ParseKeymapLayers(config_buffer_t *buffer, keymap_t keymap, uint8_t keymapCount, uint8_t macroCount)
{
    uint16_t layerCount = ReadCompactLength(buffer);

    if (layerCount != LayerId_Count) {
        return ParserError_InvalidLayerCount;
    }
    for (uint8_t layerIdx = 0; layerIdx < layerCount; layerIdx++) {
        parser_error_t errorCode = parseLayer(buffer, keymap == NULL ? NULL : keymap[layerIdx], keymapCount, macroCount);
        if (errorCode != ParserError_Success) {
            return errorCode;
        }
    }
    return ParserError_Success;
}

parser_error_t ParseKeymap(config_buffer_t *buffer, uint8_t keymapIdx, uint8_t keymapCount, uint8_t macroCount, bool applyConfig)
{
    parser_error_t errorCode;
    uint16_t abbreviationLen;
    uint16_t nameLen;
    uint16_t descriptionLen;
    const char *abbreviation = ReadString(buffer, &abbreviationLen);
    bool isDefault = ReadBool(buffer);
    const char *name = ReadString(buffer, &nameLen);
    const char *description = ReadString(buffer, &descriptionLen);

    (void)name;
    (void)description;
    if (!abbreviationLen || abbreviationLen > 3) {
        return ParserError_InvalidAbbreviationLen;
    }

    uint16_t layersStartOffset = buffer->offset;
    // Parse layers for errors, but do not apply.  
    // Keymaps are reparsed applied with SwitchKeymap*
    errorCode = ParseKeymapLayers(buffer, NULL, keymapCount, macroCount);
    if (errorCode != ParserError_Success) {
        return errorCode;
    }
    if (applyConfig) {
        AllKeymaps[keymapIdx].abbreviation = abbreviation;
        AllKeymaps[keymapIdx].abbreviationLen = abbreviationLen;
        AllKeymaps[keymapIdx].offset = layersStartOffset;
        if (isDefault) {
            DefaultKeymapIndex = keymapIdx;
        }
    }
    return ParserError_Success;
}
