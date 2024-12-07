#pragma bank 255

#include "vm_rtc.h"

#include "vm.h"

#include "rtc.h"

BANKREF(VM_RTC)

void vm_rtc_latch(void) OLDCALL BANKED {
#if defined(GAMEBOY)
    //RTC_LATCH();
#endif
}

void vm_rtc_get(SCRIPT_CTX * THIS, INT16 idx, UBYTE what) OLDCALL BANKED {
    idx;
    INT16 * res = VM_REF_TO_PTR(idx);
#if defined(GAMEBOY)
    *res = RTC_GET((rtc_dateparts_e)((what & 0x03) + RTC_VALUE_SEC));
#else
    *res = 0x1337;
#endif
}

void vm_rtc_set(SCRIPT_CTX * THIS, INT16 idx, UBYTE what) OLDCALL BANKED {
    THIS; idx; what;
#if defined(GAMEBOY)
    INT16 * res = VM_REF_TO_PTR(idx);
    RTC_SET((rtc_dateparts_e)((what & 0x03) + RTC_VALUE_SEC), (UWORD)*res);
#endif
}

void vm_rtc_start(SCRIPT_CTX * THIS, UBYTE start) OLDCALL BANKED {
    THIS;
#if defined(GAMEBOY)
    RTC_START(start);
#endif
}
