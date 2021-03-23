#include "i2c_addresses.h"
#include "i2c.h"
#include "slave_scheduler.h"
#include "slave_drivers/uhk_module_driver.h"
#include "slave_protocol.h"
#include "peripherals/test_led.h"
#include "bool_array_converter.h"
#include "crc16.h"
#include "key_states.h"
#include "usb_interfaces/usb_interface_mouse.h"
#include "touchpad_driver.h"

void TouchpadDriver_Init(uint8_t uhkModuleDriverId)
{
}

typedef struct {
    struct {
        bool singleTap: 1;
        bool tapAndHold: 1;
        bool swipeXN: 1;
        bool swipeXP: 1;
        bool swipeYP: 1;
        bool swipeYN: 1;
        uint8_t unused: 2;
    } events0;
    struct {
        bool twoFingerTap : 1;
        bool scroll : 1;
        bool zoom : 1;
        uint8_t unused: 5;
    } events1;
} gesture_events_t;

static gesture_events_t gestureEvents;

uint8_t address = I2C_ADDRESS_RIGHT_IQS5XX_FIRMWARE;
touchpad_events_t TouchpadEvents;
uint8_t phase = 0;
static uint8_t enableEventMode[] = {0x05, 0x8f, 0x07};
static uint8_t getGestureEvents0[] = {0x00, 0x0d};
static uint8_t getRelativePixelsXCommand[] = {0x00, 0x12};
static uint8_t closeCommunicationWindow[] = {0xee, 0xee, 0xee};
static uint8_t buffer[4];
int16_t deltaX;
int16_t deltaY;

status_t TouchpadDriver_Update(uint8_t uhkModuleDriverId)
{
    status_t status = kStatus_Uhk_IdleSlave;

    switch (phase) {
        case 0: {
            status = I2cAsyncWrite(address, enableEventMode, sizeof(enableEventMode));
            phase = 1;
            break;
        }
        case 1: {
            status = I2cAsyncWrite(address, getGestureEvents0, sizeof(getGestureEvents0));
            phase = 2;
            break;
        }
        case 2: {
            status = I2cAsyncRead(address, (uint8_t*)&gestureEvents, sizeof(gesture_events_t));
            phase = 3;
            break;
        }
        case 3: {
            status = I2cAsyncWrite(address, getRelativePixelsXCommand, sizeof(getRelativePixelsXCommand));
            phase = 4;
            break;
        }
        case 4: {
            status = I2cAsyncRead(address, buffer, 4);
            phase = 5;
            break;
        }
        case 5: {
            deltaY = (int16_t)(buffer[1] | buffer[0]<<8);
            deltaX = (int16_t)(buffer[3] | buffer[2]<<8);

            KeyStates[uhkModuleDriverId+1][TouchpadKeyId_LeftButton].hardwareSwitchState |= gestureEvents.events0.singleTap;
            KeyStates[uhkModuleDriverId+1][TouchpadKeyId_RightButton].hardwareSwitchState |= gestureEvents.events1.twoFingerTap;
            KeyStates[uhkModuleDriverId+1][TouchpadKeyId_TapAndHold].hardwareSwitchState |= gestureEvents.events0.tapAndHold;
            KeyStates[uhkModuleDriverId+1][TouchpadKeyId_SwipeUp].hardwareSwitchState |= gestureEvents.events0.swipeYP;
            KeyStates[uhkModuleDriverId+1][TouchpadKeyId_SwipeDown].hardwareSwitchState |= gestureEvents.events0.swipeYN;
            KeyStates[uhkModuleDriverId+1][TouchpadKeyId_SwipeRight].hardwareSwitchState |= gestureEvents.events0.swipeXP;
            KeyStates[uhkModuleDriverId+1][TouchpadKeyId_SwipeLeft].hardwareSwitchState |= gestureEvents.events0.swipeXN;

            TouchpadEvents.pointer.delta.x -= deltaX;
            TouchpadEvents.pointer.delta.y += deltaY;
            TouchpadEvents.pointer.abs.x = -deltaX;
            TouchpadEvents.pointer.abs.y = deltaY;

            bool cursor = !gestureEvents.events1.scroll && !gestureEvents.events1.zoom;
            KeyStates[uhkModuleDriverId+1][TouchpadKeyId_MouseMove].hardwareSwitchState = cursor;
            KeyStates[uhkModuleDriverId+1][TouchpadKeyId_MoveScroll].hardwareSwitchState = gestureEvents.events1.scroll;
            KeyStates[uhkModuleDriverId+1][TouchpadKeyId_MoveZoom].hardwareSwitchState = gestureEvents.events1.zoom;

            status = I2cAsyncWrite(address, closeCommunicationWindow, sizeof(closeCommunicationWindow));
            phase = 1;
            break;
        }
    }

    return status;
}

void TouchpadDriver_Disconnect(uint8_t uhkModuleDriverId)
{
    TouchpadEvents.pointer.delta.x = 0;
    TouchpadEvents.pointer.delta.y = 0;
    memset(KeyStates[uhkModuleDriverId+1], 0, MAX_KEY_COUNT_PER_MODULE * sizeof(key_state_t));
}
