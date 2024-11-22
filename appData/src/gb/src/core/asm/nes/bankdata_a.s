.include "global.s"

.area GBDKOVR (PAG, OVR)
;_MoveMetaspriteBanked_PARM_2::
_SetBankedSpriteData_PARM_3::
_SetBankedBkgData_PARM_3::          .ds 2
;_MoveMetaspriteBanked_PARM_3::
_SetBankedSpriteData_PARM_4::
_SetBankedBkgData_PARM_4::          .ds 2
;_MoveMetaspriteBanked_PARM_4::
_SetBankedBkgData_padding:          .ds 3   ; Pad overlay segment to not conflict with set_data / set_sprite_data locals
;_MoveMetaspriteBanked_bank_save::
_SetBankedBkgData_bank_save::
_SetBankedSpriteData_bank_save::    .ds 1
_ReadBankedUBYTE_PARM_2::
_ReadBankedUWORD_PARM_2::           .ds 2

.area OSEG (PAG, OVR)
.vm_replace_tile_xy_impl_PARM_x::
_vm_replace_tile_xy_impl_PARM_2::   .ds 1
.vm_replace_tile_xy_impl_PARM_y::
_vm_replace_tile_xy_impl_PARM_3::   .ds 1
.vm_replace_tile_xy_impl_PARM_tileset_bank::
_vm_replace_tile_xy_impl_PARM_4::   .ds 1
.vm_replace_tile_xy_impl_PARM_tileset::
_vm_replace_tile_xy_impl_PARM_5::   .ds 2
.vm_replace_tile_xy_impl_PARM_idx_start_tile::
_vm_replace_tile_xy_impl_PARM_6::   .ds 2

; void vm_replace_tile_xy_impl(SCRIPT_CTX * THIS, UBYTE x, UBYTE y_, UBYTE tileset_bank, const tileset_t * tileset, INT16 idx_start_tile)

.area _HOME

_ReadBankedUWORD::
    sta *REGTEMP
    stx *REGTEMP+1
    lda *__current_bank
    sta *REGTEMP+4
    lda *_ReadBankedUWORD_PARM_2
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

_ReadBankedUBYTE::
    sta *REGTEMP
    stx *REGTEMP+1
.ReadBankedUBYTE_REGTEMP:
    lda *__current_bank
    sta *REGTEMP+2
    lda *_ReadBankedUBYTE_PARM_2
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

_SetBankedBkgData::
    sta *REGTEMP
    lda *__current_bank
    sta *_SetBankedBkgData_bank_save
    lda *_SetBankedBkgData_PARM_4
    sta *__current_bank
    tay
    sta .identity,y
    lda *REGTEMP
    jsr _set_bkg_data
    lda *_SetBankedBkgData_bank_save
    sta *__current_bank
    tay
    sta .identity,y
    rts

_SetBankedSpriteData::
    pha
    lda *__current_bank
    sta *_SetBankedSpriteData_bank_save
    lda *_SetBankedSpriteData_PARM_4
    jsr __switch_prg0
    pla
    jsr _set_sprite_data
    lda *_SetBankedSpriteData_bank_save
    jmp __switch_prg0




; void vm_replace_tile_xy_impl(SCRIPT_CTX * THIS, UBYTE x, UBYTE y_, UBYTE tileset_bank, const tileset_t * tileset, INT16 idx_start_tile)

_vm_replace_tile_xy_impl::
    VM_HEAP_SIZE = 128
    ;
    sta *REGTEMP
    stx *REGTEMP+1
    ; = tileset + 16*stack_ptr[idx_start_tile]
    ; TODO: Check if we need to support heap access as well as stack
    lda *.vm_replace_tile_xy_impl_PARM_idx_start_tile
    asl
    ldy #08
    clc
    adc [*REGTEMP],y
    tax
    lda _script_memory+2*VM_HEAP_SIZE,x
    tay
    lsr
    lsr
    lsr
    lsr
    sta *REGTEMP
    tya
    asl
    asl
    asl
    asl    
    ora #2  ; skip n_tiles
    clc
    adc *.vm_replace_tile_xy_impl_PARM_tileset
    sta *_SetBankedBkgData_PARM_3
    lda *REGTEMP
    adc *.vm_replace_tile_xy_impl_PARM_tileset+1
    sta *_SetBankedBkgData_PARM_3+1
    lda *.vm_replace_tile_xy_impl_PARM_tileset_bank
    sta *_SetBankedBkgData_PARM_4
    ;
    ldy *.vm_replace_tile_xy_impl_PARM_y
    lda _image_row_addr_lo,y
    clc
    adc *.vm_replace_tile_xy_impl_PARM_x
    sta *REGTEMP
    lda _image_row_addr_hi,y
    adc #0
    sta *REGTEMP+1
    lda _image_bank
    sta *_ReadBankedUBYTE_PARM_2
    jsr .ReadBankedUBYTE_REGTEMP
    ldx #1
    jmp _SetBankedBkgData
