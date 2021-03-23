#ifndef __SLAVE_DRIVER_TOUCHPAD_MODULE_H__
#define __SLAVE_DRIVER_TOUCHPAD_MODULE_H__

// Includes:

    #include "fsl_common.h"
    #include "crc16.h"
    #include "versions.h"
    #include "slot.h"
    #include "usb_interfaces/usb_interface_mouse.h"

// Defines:

    typedef enum {
        TouchpadKeyId_LeftButton,
        TouchpadKeyId_RightButton,
        TouchpadKeyId_MiddleButton,
        TouchpadKeyId_MouseMove,
        TouchpadKeyId_TapAndHold,
        TouchpadKeyId_SwipeUp,
        TouchpadKeyId_SwipeDown,
        TouchpadKeyId_SwipeLeft,
        TouchpadKeyId_SwipeRight,
        TouchpadKeyId_MoveScroll,
        TouchpadKeyId_MoveZoom,
    } touchpad_keyid_t;

// Typedefs:

    typedef enum {
        TouchpadDriverId_Singleton,
    } touchpad_driver_id_t;

    typedef struct {
        pointer_data_t pointer;
        int8_t wheelY;
        int8_t wheelX;
        int8_t zoomLevel;
    } touchpad_events_t;

// Variables:

    extern touchpad_events_t TouchpadEvents;

// Functions:

    void TouchpadDriver_Init(uint8_t uhkModuleDriverId);
    status_t TouchpadDriver_Update(uint8_t uhkModuleDriverId);
    void TouchpadDriver_Disconnect(uint8_t uhkModuleDriverId);

#endif
