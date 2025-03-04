#ifndef CAMERA_H
#define CAMERA_H

#include <gbdk/platform.h>

#define SCREEN_WIDTH 256 //160
#define SCREEN_HEIGHT 240 //144
#define SCREEN_WIDTH_HALF 128 //80
#define SCREEN_HEIGHT_HALF 120 //72

#define CAMERA_UNLOCKED 0x00
#define CAMERA_LOCK_X_FLAG 0x01
#define CAMERA_LOCK_Y_FLAG 0x02
#define CAMERA_LOCK_FLAG (CAMERA_LOCK_X_FLAG | CAMERA_LOCK_Y_FLAG)

extern INT16 camera_x;
extern INT16 camera_y;
extern INT16 camera_offset_x_x16;
extern INT16 camera_offset_y_x16;
extern UWORD camera_deadzone_x_x16;
extern UWORD camera_deadzone_y_x16;
extern UBYTE camera_settings;

void camera_init(void) BANKED;

inline void camera_reset(void) {
    camera_deadzone_x_x16 = camera_deadzone_y_x16 = 0;
}

void camera_update(void) BANKED;

#endif