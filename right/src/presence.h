#ifndef __PRESENCE_H__
#define __PRESENCE_H__

// Includes:

    #include <stdbool.h>
    #include "key_action.h"

// Defines
    #define PRESENCE_MAX_PASSWORD 10

// Typedefs
    typedef void (*presence_callback_t) (void);

// Functions:
    void Presence_Init(presence_callback_t *callback, uint8_t textLen, const char*text, bool secure);
    void Presence_Config(uint8_t passwordLen, const char* password);
    void Presence_Continue(key_action_t * action);
    void Presence_Cancel();

// Variables:
    extern uint8_t PresencePasswordScancodes[PRESENCE_MAX_PASSWORD];
#endif