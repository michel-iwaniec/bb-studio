.include "global.s"

.area _CODE

.globl _state_start_fns, _state_update_fns

.globl _scene_type
.globl __current_bank

.define .l "REGTEMP+2"
.define .h "REGTEMP+3"
.define .hl "REGTEMP+2"

_state_init::
        lda #<_state_start_fns
        sta *.l
        lda #>_state_start_fns
        sta *.h
        jmp state_call

_state_update::
        lda #<_state_update_fns
        sta *.l
        lda #>_state_update_fns
        sta *.h

state_call:
        ; hl += _scenetype * sizeof(far_ptr)
        lda _scene_type
        sta *REGTEMP
        asl
        adc *REGTEMP
        clc
        adc *.l
        sta *.l
        lda #0
        adc *.h
        sta *.h

        ; save current bank
        lda __current_bank
        pha

        ; load and switch to bank
        ldy #0        
        lda [*.hl],y
        jsr __switch_prg0

        ; load offset
        ldy #1
        lda [*.hl],y
        iny
        sta *REGTEMP
        lda [*.hl],y
        sta *REGTEMP+1
        
        ; call REGTEMP
        jsr __sdcc_indirect_jsr

        ; restore bank
        pla
        jsr __switch_prg0
        rts
