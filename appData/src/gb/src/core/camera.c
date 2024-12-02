#pragma bank 255

#include "camera.h"

#include "actor.h"

#define CAMERA_FIXED_OFFSET_X 128
#define CAMERA_FIXED_OFFSET_Y 128

INT16 camera_x;
INT16 camera_y;
INT16 camera_offset_x_x16;
INT16 camera_offset_y_x16;
UWORD camera_deadzone_x_x16;
UWORD camera_deadzone_y_x16;
UBYTE camera_settings;

void camera_init(void) BANKED {
    camera_settings = CAMERA_LOCK_FLAG;
    camera_x = camera_y = 0;
    camera_offset_x_x16 = camera_offset_y_x16 = 0;
    camera_reset();
}

void camera_update(void) BANKED {
    if ((camera_settings & CAMERA_LOCK_X_FLAG)) {
        UWORD a_x = PLAYER.pos.x + CAMERA_FIXED_OFFSET_X;
        // Horizontal lock
        if (camera_x < a_x - camera_deadzone_x_x16 - camera_offset_x_x16) { 
            camera_x = a_x - camera_deadzone_x_x16 - camera_offset_x_x16;
        } else if (camera_x > a_x + camera_deadzone_x_x16 - camera_offset_x_x16) { 
            camera_x = a_x + camera_deadzone_x_x16 - camera_offset_x_x16;
        }
    }

    if ((camera_settings & CAMERA_LOCK_Y_FLAG)) {
        UWORD a_y = PLAYER.pos.y + CAMERA_FIXED_OFFSET_Y;
        // Vertical lock
        if (camera_y < a_y - camera_deadzone_y_x16 - camera_offset_y_x16) { 
            camera_y = a_y - camera_deadzone_y_x16 - camera_offset_y_x16;
        } else if (camera_y > a_y + camera_deadzone_y_x16 - camera_offset_y_x16) { 
            camera_y = a_y + camera_deadzone_y_x16 - camera_offset_y_x16;
        }
    }
}
