
#ifndef __RATIONAL_H__
#define __RATIONAL_H__

#include <stdint.h>
#include "attributes.h"

typedef 
union {
    struct {
        uint8_t top;
        uint8_t bottom;
    } ATTR_PACKED parts;
    uint16_t asUint16;
} rational_t;

inline int16_t multRational(int16_t value, uint8_t top, uint8_t bottom) {
    return (((int32_t) value) * top) / bottom;
}
#endif 