.include "global.s"

.area _BSS
_collision_row_addr_lo::    .ds 128
_collision_row_addr_hi::    .ds 128
;_image_row_offs_lo::    .ds 128
;_image_row_offs_hi::    .ds 128
_image_row_addr_lo::    .ds 128
_image_row_addr_hi::    .ds 128

.area _CODE

_tile_at::
        tay
        ; Fetch address
        lda _collision_row_addr_lo,x
        sta *REGTEMP
        lda _collision_row_addr_hi,x
        sta *REGTEMP+1
        ; Save old bank and switch to collision map bank
        lda *__current_bank
        sta *REGTEMP+2
        lda _collision_bank
        tax
        sta *__current_bank
        sta .identity,x
        ; Read banked byte, save in X
        lda [*REGTEMP],y
        tax
        ; Restore saved bank
        lda *REGTEMP+2
        tay
        sta *__current_bank
        sta .identity,y
        ; Return banked byte
        txa
        rts
