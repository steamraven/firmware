#include "parse_extensions.h"
#include "config_globals.h"
#include "extensions.h"
#include "presence.h"
#include "macros.h"

parser_error_t ParseExtensions(config_buffer_t *buffer, bool applyConfig) {
    
    uint16_t extensionCount = ReadCompactLength(buffer);

    for (uint16_t extensionIdx=0; extensionIdx<extensionCount; extensionIdx++) {
        uint16_t extensionLength = ReadCompactLength(buffer);
        uint16_t maxOffset = buffer->offset + extensionLength;

        uint16_t extensionNameLen;
        const char* extensionName = ReadString(buffer, &extensionNameLen);
        
        /* Each extension supported by this firmware should have a case 
           and skip unknown extensions */

        /* Example extension parser.
        if (strncmp("Namespace/Example", extensionName, extensionNameLen) == 0) {
            extensions_example_t example;
 
            example.field1 = ReadUInt8(buffer);
            
            // validation
            if (example.field1 == 0) {
                return ParseError_InvalidExtensionValue
            }

            // Field added after first version
            if (buffer->offset < maxOffset) {
                example.laterField2 = ReadUInt16(buffer);
            } else {
                example.field2 = 5; // default if not in config
            }

            if (applyConfig) {
                Extensions.example = example;
            }
        } else if (...) {} 
        */
       if (strncmp("steamraven/secure-presence", extensionName, extensionNameLen) == 0) {
           if (buffer->offset < maxOffset) {
                uint16_t passwordLen = 0;
                const char *password = ReadString(buffer, &passwordLen);
                if (passwordLen > PRESENCE_MAX_PASSWORD) {
                    return ParserError_InvalidExtensionValue;
                }
                uint8_t i;
                for (i=0; i < passwordLen; i++) {
                    uint8_t scancode = CharacterToScancode(password[i]);
                    if (scancode == 0) {
                        return ParserError_InvalidExtensionValue;
                    }
                    PresencePasswordScancodes[i] = scancode;
                }
                if (i < PRESENCE_MAX_PASSWORD) {
                    PresencePasswordScancodes[i] = 0;
                }    
           }
       }

       buffer->offset = maxOffset;
    }

    return ParserError_Success;
}