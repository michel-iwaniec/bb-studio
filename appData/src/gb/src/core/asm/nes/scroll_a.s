.include "global.s"

.area _CODE

_GetWinAddr::
        lda #<0x2000
        ldx #>0x2000
        rts

_GetBkgAddr::
        lda #<0x2000
        ldx #>0x2000
        rts

.area _CODE_255

; void scroll_rect(UBYTE * base_addr, UBYTE w, UBYTE h, UBYTE fill) BANKED; 
.globl b_scroll_rect
b_scroll_rect = 255

_scroll_rect::
        rts
