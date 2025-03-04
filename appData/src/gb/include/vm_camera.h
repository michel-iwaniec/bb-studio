#ifndef _VM_CAMERA_H_INCLUDE
#define _VM_CAMERA_H_INCLUDE

#include <gbdk/platform.h>

#include "vm.h"

#define CAMERA_SHAKE_X 1
#define CAMERA_SHAKE_Y 2

BANKREF_EXTERN(VM_CAMERA)

void vm_camera_move_to(SCRIPT_CTX * THIS, INT16 idx, UBYTE speed, UBYTE after_lock_camera) OLDCALL BANKED REENTRANT;
void vm_camera_set_pos(SCRIPT_CTX * THIS, INT16 idx) OLDCALL BANKED REENTRANT;

#endif
