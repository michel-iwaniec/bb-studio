#ifndef _VM_MUSIC_H_INCLUDE
#define _VM_MUSIC_H_INCLUDE

#include <gbdk/platform.h>

#include "vm.h"
#include "music_manager.h"

BANKREF_EXTERN(VM_MUSIC)

void vm_music_play(SCRIPT_CTX * THIS, UBYTE track_bank, const TRACK_T *track) OLDCALL BANKED REENTRANT;
void vm_music_stop(DUMMY_SIGNATURE) OLDCALL BANKED REENTRANT;
void vm_music_mute(SCRIPT_CTX * THIS, UBYTE channels) OLDCALL BANKED REENTRANT;
void vm_music_routine(SCRIPT_CTX * THIS, UBYTE routine, UBYTE bank, UBYTE * pc) OLDCALL BANKED REENTRANT;
void vm_music_setpos(SCRIPT_CTX * THIS, UBYTE pattern, UBYTE row) OLDCALL BANKED REENTRANT;

void vm_sound_mastervol(SCRIPT_CTX * THIS, UBYTE volume) OLDCALL BANKED REENTRANT;

void vm_sfx_play(SCRIPT_CTX * THIS, UBYTE bank, UBYTE * offset, UBYTE channel_mask, UBYTE priority) OLDCALL BANKED REENTRANT;

#endif