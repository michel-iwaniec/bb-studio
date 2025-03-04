#ifndef _VM_SIO_H_INCLUDE
#define _VM_SIO_H_INCLUDE

#include <gbdk/platform.h>

#include "vm.h"

BANKREF_EXTERN(VM_SIO)

void vm_sio_set_mode(SCRIPT_CTX * THIS, UBYTE mode) OLDCALL BANKED REENTRANT;
void vm_sio_exchange(SCRIPT_CTX * THIS, INT16 idxA, INT16 idxB, UBYTE len) OLDCALL BANKED REENTRANT;

#endif