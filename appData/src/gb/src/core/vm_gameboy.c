#pragma bank 255

#include "vm_gameboy.h"

#include <gbdk/metasprites.h>

#include "system.h"
#include "vm.h"

#include "input.h"
#include "events.h"
#include "fade_manager.h"
#include "music_manager.h"
#include "load_save.h"
#include "bankdata.h"
#include "data_manager.h"
#include "interrupts.h"

BANKREF(VM_GAMEBOY)

void vm_set_sprites_visible(SCRIPT_CTX * THIS, UBYTE mode) OLDCALL BANKED REENTRANT {
    THIS;
    if (hide_sprites = mode)
    {
        SHOW_SPRITES;
    }
    else
    {
        HIDE_SPRITES;
    }
}

void vm_input_wait(SCRIPT_CTX * THIS, UBYTE mask) OLDCALL BANKED REENTRANT {
    if ((joy != last_joy) && (joy & mask)) return;
    THIS->waitable = 1;
    THIS->PC -= INSTRUCTION_SIZE + sizeof(mask);
}

void vm_context_prepare(SCRIPT_CTX * THIS, UBYTE slot, UBYTE bank, UBYTE * pc) OLDCALL BANKED REENTRANT {
    THIS;
    script_event_t * event = &input_events[(slot - 1) & 7];
    event->script_bank = bank;
    event->script_addr = pc;
}

void vm_input_attach(SCRIPT_CTX * THIS, UBYTE mask, UBYTE slot) OLDCALL BANKED REENTRANT {
    THIS;
    UBYTE * current_slot = input_slots;
    for (UBYTE tmp = mask; (tmp); tmp = tmp >> 1, current_slot++) {
        if (tmp & 1) *current_slot = slot;
    }
}

void vm_input_detach(SCRIPT_CTX * THIS, UBYTE mask) OLDCALL BANKED REENTRANT {
    THIS;
    UBYTE * current_slot = input_slots;
    for (UBYTE tmp = mask; (tmp); tmp = tmp >> 1, current_slot++) {
        if (tmp & 1) *current_slot = 0;
    }
}

void vm_input_get(SCRIPT_CTX * THIS, INT16 idx, UBYTE joyid) OLDCALL BANKED REENTRANT {
    INT16 * A = VM_REF_TO_PTR(idx);
    *A = joypads.joypads[joyid];
}

void vm_fade(SCRIPT_CTX * THIS, UBYTE mode) OLDCALL BANKED REENTRANT {
    THIS;
    if (mode & FADE_DIR_IN) {
        if (mode & FADE_MODE_MODAL) fade_in_modal(); else fade_in();
    } else {
        if (mode & FADE_MODE_MODAL) fade_out_modal(); else fade_out();
    }
}

void vm_timer_prepare(SCRIPT_CTX * THIS, UBYTE timer, UBYTE bank, UBYTE * pc) OLDCALL BANKED REENTRANT {
    THIS;
    script_event_t * event = &timer_events[(timer - 1) & 3];
    event->script_bank = bank;
    event->script_addr = pc;
}

void vm_timer_set(SCRIPT_CTX * THIS, UBYTE timer, UBYTE value) OLDCALL BANKED REENTRANT {
    THIS;
    timer_time_t * timer_value = &timer_values[(timer - 1) & 3];
    timer_value->value = value;
    timer_value->remains = value;
}

void vm_timer_stop(SCRIPT_CTX * THIS, UBYTE timer) OLDCALL BANKED REENTRANT {
    THIS;
    timer_time_t * timer_value = &timer_values[(timer - 1) & 3];
    timer_value->value = 0;
}

void vm_timer_reset(SCRIPT_CTX * THIS, UBYTE timer) OLDCALL BANKED REENTRANT {
    THIS;
    timer_time_t * timer_value = &timer_values[(timer - 1) & 3];
    timer_value->remains = timer_value->value;
}

void vm_get_tile_xy(SCRIPT_CTX * THIS, INT16 idx_tile, INT16 idx_x, INT16 idx_y) OLDCALL BANKED REENTRANT {
    THIS;

    INT16 * res = VM_REF_TO_PTR(idx_tile);
    INT16 * X = VM_REF_TO_PTR(idx_x);
    INT16 * Y = VM_REF_TO_PTR(idx_y);

    UWORD ofs = (image_tile_width * ((UBYTE)*Y)) + ((UBYTE)*X);
    UBYTE target_tile = ReadBankedUBYTE(image_ptr + ofs, image_bank);
#ifdef CGB
    if (_is_CGB) {
        UBYTE tartet_attr = ReadBankedUBYTE(image_attr_ptr + ofs, image_attr_bank);
        *res = (((UWORD)tartet_attr) << 8) | target_tile;
        return;
    }
#endif
    *res = target_tile;
}

void vm_replace_tile(SCRIPT_CTX * THIS, INT16 idx_target_tile, UBYTE tileset_bank, const tileset_t * tileset, INT16 idx_start_tile, UBYTE length) OLDCALL BANKED REENTRANT {
    INT16 * A = VM_REF_TO_PTR(idx_start_tile);
    INT16 * B = VM_REF_TO_PTR(idx_target_tile);
#ifdef CGB
    //gbdk-nes TODO: attribute if (_is_CGB) VBK_REG =  (*B & 0x0800) ? 1 : 0;
#endif
    SetBankedBkgData((UBYTE)(*B), length, tileset->tiles + (*A << 4), tileset_bank);
#ifdef CGB
    //VBK_REG = 0;
#endif
}

void vm_poll(SCRIPT_CTX * THIS, INT16 idx, INT16 res, UBYTE event_mask) OLDCALL BANKED REENTRANT {
    INT16 * result_mask = VM_REF_TO_PTR(idx);
    INT16 * result = VM_REF_TO_PTR(res);
    if (event_mask & POLL_EVENT_INPUT) {
        if (joy != last_joy) {
            *result_mask = POLL_EVENT_INPUT;
            *result = joy;
            return;
        }
    }
    if (event_mask & POLL_EVENT_MUSIC) {
        UBYTE poll_res = music_events_poll();
        if (poll_res) {
            *result_mask = POLL_EVENT_MUSIC;
            *result = poll_res;
            return;
        }
    }
    THIS->waitable = 1;
    THIS->PC -= INSTRUCTION_SIZE + sizeof(idx) + sizeof(res) + sizeof(event_mask);
}

void vm_set_sprite_mode(SCRIPT_CTX * THIS, UBYTE mode) OLDCALL BANKED REENTRANT {
    THIS;
    if (mode)
    {
        SPRITES_8x16;
    }
    else
    {
        SPRITES_8x8;
    }
}

extern UBYTE image_row_addr_lo[128];
extern UBYTE image_row_addr_hi[128];


void vm_replace_tile_xy_impl(SCRIPT_CTX * THIS, UBYTE x, UBYTE y_, UBYTE tileset_bank, const tileset_t * tileset, INT16 idx_start_tile);

/*
{
    THIS; x; y_; tileset_bank; tileset; idx_start_tile;
__asm
    ;VM_HEAP_SIZE = 128
    ;
    sta *REGTEMP+4
    stx *REGTEMP+5
    
    ;sta *REGTEMP
    lda _vm_replace_tile_xy_impl_PARM_3 ;txa
    tay
    lda _image_row_addr_lo,y
    clc
    adc _vm_replace_tile_xy_impl_PARM_2 ;*REGTEMP
    pha
    lda _image_row_addr_hi,y
    adc #0
    tax
    lda _image_bank
    sta _ReadBankedUWORD_PARM_2
    pla
    jsr _ReadBankedUWORD
    pha
    ; = tileset + 16*stack_ptr[idx_start_tile]
    ; TODO: Check if we need to support heap access as well as stack
      
    
    asl _vm_replace_tile_xy_impl_PARM_6

    ldy #08
    lda [*REGTEMP+4],y
    clc
    adc _vm_replace_tile_xy_impl_PARM_6
    tax

    ;ldx _vm_replace_tile_xy_impl_PARM_6
    lda _script_memory+2*VM_HEAP_SIZE+1,x
    ;lda #0
    sta *REGTEMP+1
    lda _script_memory+2*VM_HEAP_SIZE,x ;lda _vm_replace_tile_xy_impl_PARM_6
    asl
    rol *REGTEMP+1
    asl
    rol *REGTEMP+1
    asl
    rol *REGTEMP+1
    asl
    rol *REGTEMP+1
    clc
    adc _vm_replace_tile_xy_impl_PARM_5
    sta _SetBankedBkgData_PARM_3
    lda *REGTEMP+1
    adc _vm_replace_tile_xy_impl_PARM_5+1
    sta _SetBankedBkgData_PARM_3+1
    lda _vm_replace_tile_xy_impl_PARM_4
    sta _SetBankedBkgData_PARM_4
    pla
    ldx #1
    jsr _SetBankedBkgData
    rts
__endasm;
}
*/


void vm_replace_tile_xy(SCRIPT_CTX * THIS, UBYTE x, UBYTE y, UBYTE tileset_bank, const tileset_t * tileset, INT16 idx_start_tile) OLDCALL BANKED REENTRANT {
    //THIS;

    vm_replace_tile_xy_impl(THIS, x, y, tileset_bank, tileset, idx_start_tile);

//    //UWORD ofs = (image_tile_width * y) + x;
//    //UBYTE target_tile = ReadBankedUBYTE(image_ptr + ofs, image_bank);
//    // Optimization: Use pre-calculated collision data, to avoid expensive mul
//    //UWORD ofs = (image_row_offs_hi[y]<<8) | image_row_offs_lo[y] + x;
//    UBYTE* p = (UBYTE*)((image_row_addr_hi[y]<<8) | image_row_addr_lo[y] + x);
//    UBYTE target_tile = ReadBankedUBYTE(p, image_bank);
/*
    if (scene_type == SCENE_TYPE_LOGO) {
        if (target_tile < 128) {
            ofs = ((y > 9) ? 0x8000 : 0x9000) + (target_tile << 4);
        } else {
            ofs = 0x8800 + ((target_tile - 128) << 4);
        }
        MemcpyVRAMBanked((void *)ofs, tileset->tiles + (*(UINT16 *)(VM_REF_TO_PTR(idx_start_tile)) << 4), 16, tileset_bank);
        return;
    }
*/
//#ifdef CGB
//    if (_is_CGB) {
//        // gbdk-nes TODO: attribute if (ReadBankedUBYTE(image_attr_ptr + ofs, image_attr_bank) & 0x08) VBK_REG = 1;
//    }
//#endif
//    SetBankedBkgData(target_tile, 1, tileset->tiles + (*(UINT8 *)(VM_REF_TO_PTR(idx_start_tile)) << 4), tileset_bank);
//#ifdef CGB
//    //VBK_REG = 0;
//#endif
}

void vm_rumble(SCRIPT_CTX * THIS, UBYTE enable) OLDCALL BANKED REENTRANT {
#if defined(GAMEBOY)
    THIS;
    if (enable) SWITCH_RAM_BANK(RUMBLE_ENABLE, RUMBLE_ENABLE); else  SWITCH_RAM_BANK(0, RUMBLE_ENABLE);
#endif
}

void vm_load_tileset(SCRIPT_CTX * THIS, INT16 idx, UBYTE bank, const background_t * background) OLDCALL BANKED REENTRANT {
    UBYTE base_tile = *(INT16 *)(VM_REF_TO_PTR(idx));
    far_ptr_t tileset;
#ifdef CGB
    if (_is_CGB) {
        ReadBankedFarPtr(&tileset, (void *)&(background->cgb_tileset), bank);
        if (tileset.bank) {
            // gbdk-nes TODO: attributes
            //VBK_REG = 1;
            UWORD n_tiles = ReadBankedUWORD(&((tileset_t *)(tileset.ptr))->n_tiles, tileset.bank);
            SetBankedBkgData(base_tile, n_tiles, ((tileset_t *)(tileset.ptr))->tiles, tileset.bank);
            //VBK_REG = 0;
        }
    }
#endif
    ReadBankedFarPtr(&tileset, (void *)&(background->tileset), bank);
    UWORD n_tiles = ReadBankedUWORD(&((tileset_t *)(tileset.ptr))->n_tiles, tileset.bank);
    SetBankedBkgData(base_tile, n_tiles, ((tileset_t *)(tileset.ptr))->tiles, tileset.bank);
}
