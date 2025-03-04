#pragma bank 255

#include "vm_sgb.h"

#include "vm.h"

BANKREF(VM_SGB)

void vm_sgb_transfer(DUMMY0_t dummy0, DUMMY1_t dummy1, SCRIPT_CTX * THIS) OLDCALL NONBANKED {
#if defined(GAMEBOY)
    dummy0; dummy1; // suppress warnings
    UBYTE _save = CURRENT_BANK;
    SWITCH_ROM(THIS->bank);
    sgb_transfer((void *)THIS->PC);
    THIS->PC += (*THIS->PC & 0x07) << 4;
    SWITCH_ROM(_save);
#endif
}