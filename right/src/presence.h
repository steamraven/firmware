#ifndef __PRESENCE_H__
#define __PRESENCE_H__

// Includes:

    #include <stdbool.h>
    #include "key_action.h"

// Typedefs
    typedef void (*presence_callback_t) (void);

// Functions:
    void Presence_Init(presence_callback_t *callback, uint8_t textLen, const char*text);
    void Presence_Continue(key_action_t * action);
    void Presence_Cancel();

#endif