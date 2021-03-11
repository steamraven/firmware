#include "fsl_common.h"
#include "usb_commands/usb_command_reenumerate.h"
#include "usb_protocol_handler.h"
#include "bootloader/wormhole.h"
#include "presence.h"

void UsbCommand_Reenumerate(void)
{
    Wormhole.magicNumber = 0;
    Wormhole.enumerationMode = GetUsbRxBufferUint8(1);
    Wormhole.timeoutMs       = GetUsbRxBufferUint32(2);
    Presence_Init(*reenumerateCallback, 3, "FRM");
}

static void reenumerateCallback()
{
    Wormhole.magicNumber = WORMHOLE_MAGIC_NUMBER;
    NVIC_SystemReset();
}
