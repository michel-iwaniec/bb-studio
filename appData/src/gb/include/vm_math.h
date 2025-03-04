#ifndef _VM_MATH_H_INCLUDE
#define _VM_MATH_H_INCLUDE

#include <gbdk/platform.h>

#include "vm.h"

BANKREF_EXTERN(VM_MATH)

void vm_sin_scale(SCRIPT_CTX * THIS, INT16 idx, INT16 idx_angle, UBYTE accuracy) OLDCALL BANKED REENTRANT;
void vm_cos_scale(SCRIPT_CTX * THIS, INT16 idx, INT16 idx_angle, UBYTE accuracy) OLDCALL BANKED REENTRANT;

#endif