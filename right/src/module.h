#ifndef __MODULE_H__
#define __MODULE_H__

// Includes:

    #include "layer.h"
    #include "slot.h"
    #include "slave_protocol.h"

// Macros:

    #define MAX_KEY_COUNT_PER_MODULE     64

// Typedefs:

    typedef enum {
        NavigationMode_LayerDependant = 0,
        NavigationMode_Cursor,
        NavigationMode_Scroll,
        NavigationMode_Caret,
        NavigationMode_Media,
        NavigationMode_Zoom,
    } navigation_mode_t;

    typedef struct {
        float currentSpeed; // px/ms
        float baseSpeed;
        float speed;
        float acceleration;
        navigation_mode_t navigationModes[LayerId_Count];
    } module_configuration_t;

// Variables:

    extern module_configuration_t ModuleConfigurations[ModuleId_ModuleCount];

// Functions:

    module_configuration_t* GetModuleConfiguration(module_id_t moduleId);
    bool IsModuleAttached(module_id_t moduleId);
    slot_t ModuleIdToSlotId(module_id_t moduleId);
    module_id_t SlotIdToModuleId(slot_t slotId);

#endif
