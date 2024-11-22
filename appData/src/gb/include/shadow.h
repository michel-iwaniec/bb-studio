#ifndef _SHADOW_H_INCLUDE
#define _SHADOW_H_INCLUDE

#include "actor.h"

extern volatile OAM_item_t shadow_OAM2[MAX_HARDWARE_SPRITES];
extern uint8_t max_sprites_previous;

inline void toggle_shadow_OAM(void) {
/*
    if (_shadow_OAM_base == (UBYTE)((UWORD)&shadow_OAM >> 8)) { 
        __render_shadow_OAM = (UBYTE)((UWORD)&shadow_OAM2 >> 8); 
    } else { 
        __render_shadow_OAM = (UBYTE)((UWORD)&shadow_OAM >> 8);
    }
*/
    //
    allocated_hardware_sprites = 0;
}
inline void activate_shadow_OAM(void) {
    if(max_sprites_previous > allocated_hardware_sprites)
        hide_sprites_range(allocated_hardware_sprites, max_sprites_previous);
    max_sprites_previous = allocated_hardware_sprites;
    //hide_sprites_range(allocated_hardware_sprites, MAX_HARDWARE_SPRITES);
    _shadow_OAM_base = __render_shadow_OAM;
    
    //
}

#endif