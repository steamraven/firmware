#include "kboot.h"
#include "i2c_addresses.h"

__attribute__((used, section(".BootloaderConfig"))) const bootloader_config_t BootloaderConfig = {
    .tag = BOOTLOADER_TAG,
    .enabledPeripherals = ENABLE_PERIPHERAL_I2C,
    .i2cSlaveAddress = I2C_ADDRESS_LEFT_KEYBOARD_HALF_BOOTLOADER,
    .peripheralDetectionTimeoutMs = 300,
    .clockFlags = 0xFF,    // Disable High speed mode
    .clockDivider = 0xFF,  // Use clock divider (0)
};