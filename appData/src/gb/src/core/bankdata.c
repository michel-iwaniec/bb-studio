#include <gbdk/platform.h>
#include <string.h>

#include "compat.h"
#include "bankdata.h"
#include "scroll.h"

static UBYTE _save;         // functions below are not reentrant

void SetBankedBkgTiles(UINT8 x, UINT8 y, UINT8 w, UINT8 h, const unsigned char *tiles, UBYTE bank) OLDCALL NONBANKED NAKED {
__asm
1$:
    lda #0x03
    bne 1$
__endasm;
}

void SetBankedWinTiles(UINT8 x, UINT8 y, UINT8 w, UINT8 h, const unsigned char *tiles, UBYTE bank) OLDCALL NONBANKED NAKED {
__asm
1$:
    lda #0x04
    bne 1$
__endasm;
}

void ReadBankedFarPtr(far_ptr_t * dest, const unsigned char *ptr, UBYTE bank) NONBANKED NAKED {
__asm
    sta *REGTEMP+2
    stx *REGTEMP+3
    lda __current_bank
    pha
    lda _ReadBankedFarPtr_PARM_3
    jsr __switch_prg0
    lda _ReadBankedFarPtr_PARM_2
    sta *REGTEMP
    lda _ReadBankedFarPtr_PARM_2+1
    sta *REGTEMP+1
    ldy #0
    lda [*REGTEMP],y
    sta [*REGTEMP+2],y
    iny
    lda [*REGTEMP],y
    sta [*REGTEMP+2],y
    iny
    lda [*REGTEMP],y
    sta [*REGTEMP+2],y
    pla
    jsr __switch_prg0
    rts
__endasm;
;__asm
;1$:
;    lda #0x05
;    bne 1$
__endasm;
}

/*
UWORD ReadBankedUWORD(const unsigned char *ptr, UBYTE bank) NONBANKED NAKED NO_OVERLAY_LOCALS {
__asm
    sta *REGTEMP
    stx *REGTEMP+1
    lda __current_bank
    sta *REGTEMP+4
    lda _ReadBankedUWORD_PARM_2
    tay
    sta .identity,y
    ldy #0
    lda [*REGTEMP],y
    iny
    sta *REGTEMP+2
    lda [*REGTEMP],y
    sta *REGTEMP+3
    lda *REGTEMP+4
    tay
    sta .identity,y
    lda *REGTEMP+2
    ldx *REGTEMP+3
    rts
__endasm;
}


UBYTE ReadBankedUBYTE(const unsigned char *ptr, UBYTE bank) NONBANKED NAKED NO_OVERLAY_LOCALS {
__asm
    sta *REGTEMP
    stx *REGTEMP+1
    lda *__current_bank
    sta *REGTEMP+2
    lda _ReadBankedUBYTE_PARM_2
    tay
    sta *__current_bank
    sta .identity,y
    ldy #0
    lda [*REGTEMP],y
    tax
    lda *REGTEMP+2
    tay
    sta *__current_bank
    sta .identity,y
    txa
    rts
__endasm;
}
*/

void MemcpyBanked(void* to, const void* from, size_t n, UBYTE bank) NONBANKED {
    _save = CURRENT_BANK;
    SWITCH_ROM(bank);
    memcpy(to, from, n);
    SWITCH_ROM(_save);
}

void MemcpyVRAMBanked(void* to, const void* from, size_t n, UBYTE bank) NONBANKED {
    _save = CURRENT_BANK;
    SWITCH_ROM(bank);
    // gbdk-nes-TODO: set_data(to, from, n);
    SWITCH_ROM(_save);
}

UBYTE IndexOfFarPtr(const far_ptr_t * list, UBYTE bank, UBYTE count, const far_ptr_t * item) NONBANKED REENTRANT {
    far_ptr_t v;
    for (UBYTE i = 0; i != count; i++, list++) {
        ReadBankedFarPtr(&v, (void *)list, bank);
        if ((v.bank == item->bank) && (v.ptr == item->ptr)) return i;
    }
    return count;
}



///
UBYTE find_blank_tile(const void* tiles, UBYTE bank, UBYTE l, UBYTE p0, UBYTE p1) NONBANKED NAKED {
    l; tiles; bank; p0; p1;
__asm
    sta *REGTEMP
    stx *REGTEMP+1
    lda *__current_bank;
    sta __save;
    lda _find_blank_tile_PARM_2
    jsr __switch_prg0
    lda #0
    sta *REGTEMP+2
1$:
    ldy #0
    ; Test p0,p1
2$:
    lda [*REGTEMP],y
    cmp _find_blank_tile_PARM_4
    bne 6$
    iny
    lda [*REGTEMP],y
    cmp _find_blank_tile_PARM_5
    bne 6$
    iny
    cpy #16 ;#8
    bne 2$
    ; Test p1
;3$:
;    lda [*REGTEMP],y
;    cmp _find_blank_tile_PARM_5
;    bne 6$
;    iny
;    cpy #16
;    bne 3$
    ; All rows match for both bitplanes - return with current index
    lda *REGTEMP+2
    jmp 7$
6$:
    lda *REGTEMP
    clc
    adc #16
    sta *REGTEMP
    lda *REGTEMP+1
    adc #0
    sta *REGTEMP+1
    lda *REGTEMP+2
    clc
    adc #1
    sta *REGTEMP+2
    cmp _find_blank_tile_PARM_3
    bne 1$
    lda #0
7$:
    pha
    lda __save;
    jsr __switch_prg0
    pla
    rts
__endasm;
}
