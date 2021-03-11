#include "presence.h"
#include "keymap.h"
#include "ledmap.h"
#include "led_display.h"
#include "test_switches.h"

typedef struct 
{
    uint8_t slot;
    uint8_t key;  
} password_key_t;

const static password_key_t presenceInsecureKeys[] = {{0, 14}, {0, 0}, {0, 7}, {0, 1}, {0, 8}, {0, 2}, {0, 9}, {0, 3}};
uint8_t PresencePasswordScancodes[PRESENCE_MAX_PASSWORD] = {0};

static uint8_t presenceIndex = 0;
static presence_callback_t * doneCallback;

void Presence_Init(presence_callback_t *callback, uint8_t textLen, const char *text, bool secure) {
    if (PresencePasswordScancodes[0] == 0) {
        secure = false;
    }

    doneCallback = callback;
    presenceIndex = 0;

    TestSwitches_Activate();
    LedDisplay_SetText(textLen,  text);

    for (uint8_t slot=0; slot<2; slot++) {
        for (uint8_t key=0; key <MAX_KEY_COUNT_PER_MODULE; key++) {
            key_action_t *action = &CurrentKeymap[LayerId_Base][slot][key];
            if (action->type != KeyActionType_None && secure) {
                    action->type = KeyActionType_PresenceKey;
            } else {
                action->type = KeyActionType_None;
            }
        }
    }
    if (!secure) {
        const password_key_t passKey = presenceInsecureKeys[0];
        CurrentKeymap[LayerId_Base][passKey.slot][passKey.key].type = KeyActionType_PresenceKey;
    }
    CurrentKeymap[LayerId_Base][SlotId_RightKeyboardHalf][HID_KEYBOARD_SC_BACKSPACE].type = KeyActionType_PresenceCancel;
    
    UpdateLayerLeds();
}

void Presence_Cancel() {
    doneCallback = NULL;
    Presence_End();
}

static void Presence_End() {
    SwitchKeymapById(CurrentKeymapIndex);
    if (doneCallback != NULL) {
        (*doneCallback)();
    }
    doneCallback = NULL;
}

void Presence_Continue(key_action_t * action) {
    if ( PresencePasswordScancodes[0] != 0) {
        if (action->keystroke.scancode == PresencePasswordScancodes[presenceIndex]) {
            presenceIndex++;
        } else {
            presenceIndex = 0;
        }
        if (presenceIndex >= PRESENCE_MAX_PASSWORD || PresencePasswordScancodes[presenceIndex] == 0) {
            Presence_End();
            return;
        }
    } else {
        const password_key_t passKey = presenceInsecureKeys[presenceIndex];
        key_action_t *passAction = &CurrentKeymap[LayerId_Base][passKey.slot][passKey.key];
        if (passAction == action) {
            presenceIndex++;
        } else {
            presenceIndex = 0;
        }
        if (presenceIndex >= sizeof(presenceInsecureKeys)) {
            Presence_End();
            return;
        }
        action->type = KeyActionType_None;
        const password_key_t nextPassKey = presenceInsecureKeys[presenceIndex];
        CurrentKeymap[LayerId_Base][passKey.slot][nextPassKey.key].type = KeyActionType_PresenceKey;
        UpdateLayerLeds();
    }
}