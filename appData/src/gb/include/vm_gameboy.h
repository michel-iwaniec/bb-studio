#ifndef _VM_gameboy_H_INCLUDE
#define _VM_gameboy_H_INCLUDE

#include <gbdk/platform.h>

#include "vm.h"
#include "gbs_types.h"

BANKREF_EXTERN(VM_GAMEBOY)

void vm_set_sprites_visible(SCRIPT_CTX * THIS, UBYTE mode) OLDCALL BANKED REENTRANT;
void vm_input_wait(SCRIPT_CTX * THIS, UBYTE mask) OLDCALL BANKED REENTRANT;
void vm_context_prepare(SCRIPT_CTX * THIS, UBYTE slot, UBYTE bank, UBYTE * pc) OLDCALL BANKED REENTRANT;
void vm_input_attach(SCRIPT_CTX * THIS, UBYTE mask, UBYTE slot) OLDCALL BANKED REENTRANT;
void vm_input_detach(SCRIPT_CTX * THIS, UBYTE mask) OLDCALL BANKED REENTRANT;
void vm_input_get(SCRIPT_CTX * THIS, INT16 idx, UBYTE joyid) OLDCALL BANKED REENTRANT;

#define FADE_MODE_NONMODAL 0x00
#define FADE_MODE_MODAL    0x01
#define FADE_DIR_OUT       0x00
#define FADE_DIR_IN        0x02
void vm_fade(SCRIPT_CTX * THIS, UBYTE mode) OLDCALL BANKED REENTRANT;

void vm_timer_prepare(SCRIPT_CTX * THIS, UBYTE timer, UBYTE bank, UBYTE * pc) OLDCALL BANKED REENTRANT;
void vm_timer_set(SCRIPT_CTX * THIS, UBYTE timer, UBYTE value) OLDCALL BANKED REENTRANT;
void vm_timer_stop(SCRIPT_CTX * THIS, UBYTE timer) OLDCALL BANKED REENTRANT;
void vm_timer_reset(SCRIPT_CTX * THIS, UBYTE timer) OLDCALL BANKED REENTRANT;
void vm_get_tile_xy(SCRIPT_CTX * THIS, INT16 idx_tile, INT16 idx_x, INT16 idx_y) OLDCALL BANKED REENTRANT;
void vm_replace_tile(SCRIPT_CTX * THIS, INT16 idx_target_tile, UBYTE tileset_bank, const tileset_t * tileset, INT16 idx_start_tile, UBYTE length) OLDCALL BANKED REENTRANT;

#define POLL_EVENT_INPUT 0x01
#define POLL_EVENT_MUSIC 0x02
void vm_poll(SCRIPT_CTX * THIS, INT16 idx, INT16 res, UBYTE event_mask) OLDCALL BANKED REENTRANT;

void vm_set_sprite_mode(SCRIPT_CTX * THIS, UBYTE mode) OLDCALL BANKED REENTRANT;
void vm_replace_tile_xy(SCRIPT_CTX * THIS, UBYTE x, UBYTE y, UBYTE tileset_bank, const tileset_t * tileset, INT16 idx_start_tile) OLDCALL BANKED REENTRANT;

#ifndef RUMBLE_ENABLE
#define RUMBLE_ENABLE 0x20u
#endif 
void vm_rumble(SCRIPT_CTX * THIS, UBYTE enable) OLDCALL BANKED REENTRANT;

void vm_load_tileset(SCRIPT_CTX * THIS, INT16 idx, UBYTE bank, const background_t * background) OLDCALL BANKED;
void vm_load_tileset(SCRIPT_CTX * THIS, INT16 idx, UBYTE bank, const background_t * background) OLDCALL BANKED REENTRANT;

#endif