        .include        "global.s"

        .globl _vwf_current_rotate, _vwf_current_mask, _vwf_inverse_map ;, _vwf_tile_data

        .area OSEG (PAG, OVR)
        _ui_draw_frame_row_PARM_2::     .ds 1
        _ui_draw_frame_row_PARM_3::     .ds 1
        _ui_print_shift_char_PARM_2::   .ds 2
        _ui_print_shift_char_PARM_3::   .ds 1

        ; must be in the same bank with ui.c
        .area _CODE_1

_ui_swap_tiles::
        ; Copy _vwf_tile_data+16 to _vwf_tile_data
        ldy #15
1$:
        lda _vwf_tile_data+16,y
        sta _vwf_tile_data,y
        dey
        bpl 1$
        ; Fill _vwf_tile_data+16 with _text_bkg_fill
        ldy #15
        lda _text_bkg_fill
2$:
        sta _vwf_tile_data+16,y
        dey
        bpl 2$
        rts

_ui_print_make_mask_lr::
        tay
        lda #0xFF
        sta *REGTEMP
1$:
        lsr
        dey
        bne 1$
        jmp 3$
2$:
        sec
        ror
        ror *REGTEMP
3$:
        dex
        bpl 2$
        ldx *REGTEMP
        rts

_ui_print_make_mask_rl::
10$:
    lda #0x22
    bne 10$

        .area _CODE

; used from NONBANKED code, so should be nonbanked too, because bank is random
_ui_time_masks::
        .db #0x00, #0x00, #0x01, #0x03, #0x07, #0x0f, #0x1f, #0x3f

; void ui_print_shift_char(void * dest, const void * src, UBYTE bank);

_ui_print_shift_char::
        ;.local .dest
        .define .dest "REGTEMP"
        .local .src
        .src = _ui_print_shift_char_PARM_2        
        .local .bank
        .bank = _ui_print_shift_char_PARM_3
        sta *.dest
        stx *.dest+1
        ; Save old bank and switch to new 
        lda *__current_bank
        pha
        lda .bank
        jsr __switch_prg0
        ;
        ldy #15
1$:
        ; RT = *src++ ^ _vwf_inverse_map
        lda [*.src],y
        eor _vwf_inverse_map
        ldx _vwf_current_rotate
        jsr .ui_print_shift_char_shift_CR
        sta *REGTEMP+2
        ; *dest++ = (*dest & _vwf_current_mask) | (RT & ~_vwf_current_mask)
        lda [*.dest],y
        and _vwf_current_mask
        tax
        lda _vwf_current_mask
        eor #0xFF
        and *REGTEMP+2
        ora .identity,x
        sta [*.dest],y
        dey
        bpl 1$
        ; Restore old bank and return
        pla
        jsr __switch_prg0
        rts

.ui_print_shift_char_shift_CR:
        bmi 1$
        beq 3$
0$:
        lsr
        dex
        bne 0$
        rts
1$:
        cpx #0x80
        beq 3$
2$:
        asl
        dex
        cpx #0x80
        bne 2$
3$:
        rts

; void ui_draw_frame_row(void * dest, UBYTE tile, UBYTE width);

.area _CODE_1

_ui_draw_frame_row::
.ui_draw_frame_row::
        .local .ui_draw_frame_row_tile
        .local .ui_draw_frame_row_width
        .ui_draw_frame_row_tile  = _ui_draw_frame_row_PARM_2
        .ui_draw_frame_row_width = _ui_draw_frame_row_PARM_3
        ; Set window layer for PPU stripes
        ldy #CFG_SWP_SPR_4S
        sty *__vram_transfer_mapper_bits
        ; Begin horizontal stripe
        jsr .ppu_stripe_begin_horizontal
        ; Write left-edge-tile
        lda .ui_draw_frame_row_tile
        jsr .ppu_stripe_write_byte
        ; Write middle-tile (width-2) times
        inc .ui_draw_frame_row_tile
        ldx .ui_draw_frame_row_width
        dex
        dex
1$:
        lda .ui_draw_frame_row_tile
        jsr .ppu_stripe_write_byte
        dex
        bne 1$
        ; Write right-edge-tile
        inc .ui_draw_frame_row_tile
        lda .ui_draw_frame_row_tile
        jsr .ppu_stripe_write_byte
        ; End horizontal stripe
        jsr .ppu_stripe_end
        ; Set window layer for PPU stripes
        ldy #0x00
        sty *__vram_transfer_mapper_bits
        rts
