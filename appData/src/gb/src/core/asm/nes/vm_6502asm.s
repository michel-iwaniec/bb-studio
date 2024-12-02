.include "vm.i"
.include "vm_helpers.i"

.globl b_wait_frames, _wait_frames

script_memory = 0x7800
VM_HEAP_SIZE = 128 ; 1024

;.globl _script_memory
;.globl VM_HEAP_SIZE



___bank_vm_rpn = 2
.globl ___bank_vm_rpn

    .define .vm_rpn_A               "_script_memory+2*VM_HEAP_SIZE"
    .define .vm_rpn_A_lo            "_script_memory+2*VM_HEAP_SIZE"
    .define .vm_rpn_A_hi            "_script_memory+2*VM_HEAP_SIZE+1"
    .define .vm_rpn_B               "_script_memory+2*VM_HEAP_SIZE+2"
    .define .vm_rpn_B_lo            "_script_memory+2*VM_HEAP_SIZE+2"
    .define .vm_rpn_B_hi            "_script_memory+2*VM_HEAP_SIZE+2+1"
    .define .vm_rpn_stack_idx       "_script_memory+2*VM_HEAP_SIZE+4"
    .define .vm_rpn_stack_idx_lo    "_script_memory+2*VM_HEAP_SIZE+4"
    .define .vm_rpn_stack_idx_hi    "_script_memory+2*VM_HEAP_SIZE+4+1"

    .area OSEG (PAG, OVR)
    ; THIS/CTX (passed with registers)
    .THIS_:
    .CLASH_RESERVED:    .ds 2   ; Reserve N bytes to avoid clash with OSEG parameters in __mulint, 
    .THIS:              .ds 2
    .JMP_DST:           .ds 2
    .PC:                .ds 2
    .bank:              .ds 1
    .RPN_LENGTH:        .ds 1
    .ARGS:              .ds 1

.area _CODE_2
.bndry 0x100

.vm_rpn_end:
    ; Increment PC with RPN_LENGTH + 1
    ldy #0
    lda *.RPN_LENGTH
    sec 
    adc *.PC
    sta [*.THIS],y
    iny
    lda #0
    adc *.PC+1
    sta [*.THIS],y
    ; Write stack_ptr
    inx
    inx
    inx
    inx
    STORE_STACK_IDX
    rts

_vm_rpn_asm::
.vm_rpn::
    sta *.THIS
    stx *.THIS+1
    ; Prefetch all RPN bytes and push on stack
    jmp .vm_rpn_prefetch
.vm_rpn_prefetch_done:
    ; Load stack_ptr to X, and decrement to point at potential A,B parameters
    LOAD_STACK_IDX
    stx *.ARGS
    dex
    dex
    dex
    dex

.vm_rpn_op_next:
    pla
    tay
    bmi .vm_rpn_set_op
    lda .vm_rpn_op_addr,y
    sta *.JMP_DST
    lda .vm_rpn_op_addr_hi,y
    sta *.JMP_DST+1
    lda .vm_rpn_A_lo,x
    ldy .vm_rpn_A_hi,x
    jmp [.JMP_DST]

.vm_rpn_op_next_inc_stack_ptr:
    inx
    inx
    pla
    beq .vm_rpn_end
    tay
    bmi .vm_rpn_set_op
    lda .vm_rpn_op_addr,y
    sta *.JMP_DST
    lda .vm_rpn_op_addr_hi,y
    sta *.JMP_DST+1
    lda .vm_rpn_A_lo,x
    ldy .vm_rpn_A_hi,x
    jmp [.JMP_DST]

.vm_rpn_op_next_dec_stack_ptr:
    dex
    dex
    pla
    beq .vm_rpn_end
    tay
    bmi .vm_rpn_set_op
    lda .vm_rpn_op_addr,y
    sta *.JMP_DST
    lda .vm_rpn_op_addr_hi,y
    sta *.JMP_DST+1
    lda .vm_rpn_A_lo,x
    ldy .vm_rpn_A_hi,x
    jmp [.JMP_DST]

.vm_rpn_set_op:
    jmp .vm_rpn_set_op_impl

;.MEM_I8            = 'i'
;.MEM_U8            = 'u'
;.MEM_I16           = 'I'

.define TYP "REGTEMP+2"

.vm_rpn_set_op_R_REF_MEM_SET:
    pla
    sta *TYP
    pla
    sta *REGTEMP
    pla
    sta *REGTEMP+1
    dex
    dex
    lda .vm_rpn_stack_idx_lo,x
    ldy #0
    sta [*REGTEMP],y
    lda *TYP
    cmp #'I'
    bne 1$
    ; INT16
    iny
    lda .vm_rpn_stack_idx_hi,x
    sta [*REGTEMP],y
1$:
    jmp .vm_rpn_op_next

.vm_rpn_set_op_R_REF_MEM:
    pla
    sta *TYP
    pla
    sta *REGTEMP
    pla
    sta *REGTEMP+1
    ldy #0
    lda [*REGTEMP],y
    sta .vm_rpn_stack_idx_lo,x
    ldy #1
    lda *TYP
    cmp #'I'
    beq 1$
    cmp #'i'
    beq 2$
    ; UINT8
    lda #0
    sta .vm_rpn_stack_idx_hi,x
    jmp .vm_rpn_op_next_inc_stack_ptr
2$:
    ; INT8
    lda [*REGTEMP],y
    bpl 4$
    lda #0xFF
    sta .vm_rpn_stack_idx_hi,x
    jmp .vm_rpn_op_next_inc_stack_ptr
4$:
    lda #0
    sta .vm_rpn_stack_idx_hi,x
    jmp .vm_rpn_op_next_inc_stack_ptr
1$:
    ; INT16
    lda [*REGTEMP],y
    sta .vm_rpn_stack_idx_hi,x
    jmp .vm_rpn_op_next_inc_stack_ptr
3$:
    jmp .vm_rpn_op_next_inc_stack_ptr

.vm_rpn_set_op_impl:
    iny
    beq .vm_rpn_set_op_R_INT8
    iny 
    beq .vm_rpn_set_op_R_INT16
    iny 
    beq .vm_rpn_set_op_R_REF
    iny
    beq .vm_rpn_set_op_R_REF_INDIRECT
    iny
    beq .vm_rpn_set_op_R_REF_SET
    iny
    beq .vm_rpn_set_op_R_REF_SET_IND
    iny
    beq .vm_rpn_set_op_R_REF_MEM
    iny
    beq .vm_rpn_set_op_R_REF_MEM_SET

.vm_rpn_set_op_ILLEGAL:
    jmp .vm_rpn_set_op_ILLEGAL

.vm_rpn_set_op_R_INT16:
    ; *(THIS->stack_ptr) = *((uint16_t *)(THIS->PC));
    pla
    sta .vm_rpn_stack_idx_lo,x
    pla
    sta .vm_rpn_stack_idx_hi,x
    jmp .vm_rpn_op_next_inc_stack_ptr

.vm_rpn_set_op_R_INT8:
    ; op = *(THIS->PC++);
    pla
    ; *(THIS->stack_ptr) = op;
    sta .vm_rpn_stack_idx_lo,x
    bmi 1$
    lda #0
    sta .vm_rpn_stack_idx_hi,x
    jmp .vm_rpn_op_next_inc_stack_ptr
1$:
    lda #0xFF
    sta .vm_rpn_stack_idx_hi,x
    jmp .vm_rpn_op_next_inc_stack_ptr

.vm_rpn_set_op_R_REF:
    ;idx = *((int16_t *)(THIS->PC));
    pla
    tay
    pla
    jsr .vm_rpn_set_op_eval_ptr
.vm_rpn_set_op_R_REF_write:
    ; *(THIS->stack_ptr) = *A;
    ldy #0
    lda [*REGTEMP],y
    iny
    sta .vm_rpn_stack_idx_lo,x
    lda [*REGTEMP],y
    sta .vm_rpn_stack_idx_hi,x
    jmp .vm_rpn_op_next_inc_stack_ptr

.vm_rpn_set_op_R_REF_INDIRECT:
    pla
    tay
    pla
    jsr .vm_rpn_set_op_eval_ptr
    ; idx = *((idx < 0) ? ARGS + idx : script_memory + idx);
    ldy #0
    lda [*REGTEMP],y
    tay
    lda #0
    jsr .vm_rpn_set_op_eval_ptr
    jmp .vm_rpn_set_op_R_REF_write

.vm_rpn_set_op_R_REF_SET_IND:
    ; idx = *((INT16 *)(THIS->PC));
    pla
    tay
    pla
    ; idx = *((idx < 0) ? ARGS + idx : script_memory + idx);
    jsr .vm_rpn_set_op_eval_ptr
    ; *((idx < 0) ? ARGS + idx : script_memory + idx) = *(--(THIS->stack_ptr))
    ldy #0
    lda [*REGTEMP],y
    tay
    lda #0
    jsr .vm_rpn_set_op_eval_ptr
    dex
    dex
    ldy #0
    lda .vm_rpn_stack_idx_lo,x
    sta [*REGTEMP],y
    iny
    lda .vm_rpn_stack_idx_hi,x
    sta [*REGTEMP],y
    jmp .vm_rpn_op_next

.vm_rpn_set_op_R_REF_SET:
    pla
    tay
    pla
    ; *((idx < 0) ? ARGS + idx : script_memory + idx) = *(--(THIS->stack_ptr))
    jsr .vm_rpn_set_op_eval_ptr
    dex
    dex
    ldy #0
    lda .vm_rpn_stack_idx_lo,x
    sta [*REGTEMP],y
    iny
    lda .vm_rpn_stack_idx_hi,x
    sta [*REGTEMP],y
    jmp .vm_rpn_op_next

.bndry 0x100
.vm_rpn_op_addr:
.db <.vm_rpn_op_STOP        ; R_STOP

; logical operators
.db <.vm_rpn_op_EQ          ; 1
.db <.vm_rpn_op_LT          ; 2
.db <.vm_rpn_op_LE          ; 3
.db <.vm_rpn_op_GT          ; 4
.db <.vm_rpn_op_GE          ; 5
.db <.vm_rpn_op_NE          ; 6
.db <.vm_rpn_op_AND         ; 7
.db <.vm_rpn_op_OR          ; 8
.db <.vm_rpn_op_NOT         ; 9

.db <.vm_rpn_op_ILLEGAL
.db <.vm_rpn_op_ILLEGAL
.db <.vm_rpn_op_ILLEGAL
.db <.vm_rpn_op_ILLEGAL
.db <.vm_rpn_op_ILLEGAL
.db <.vm_rpn_op_ILLEGAL

; Hard-coded SHL/SHR/ASR
.db <.vm_rpn_op_TODO        ; SHL4 (0x10)
.db <.vm_rpn_op_SHL7        ; SHL7 (0x11)
.db <.vm_rpn_op_TODO        ; SHR4 (0x12)
.db <.vm_rpn_op_TODO        ; SHR7 (0x13)
.db <.vm_rpn_op_TODO        ; ASR4 (0x14)
.db <.vm_rpn_op_ASR7        ; ASR7 (0x15)
.db <.vm_rpn_op_CLAMP0      ; CLAMP0 (0x16)

; Skip some more invisible characters
.rept 9
    .db <.vm_rpn_op_ILLEGAL
.endm

.db <.vm_rpn_op_ILLEGAL     ; ' '
.db <.vm_rpn_op_ILLEGAL     ; '!'
.db <.vm_rpn_op_ILLEGAL     ; '"'
.db <.vm_rpn_op_ILLEGAL     ; '#'
.db <.vm_rpn_op_ILLEGAL     ; '$'
.db <.vm_rpn_op_MOD         ; '%'
.db <.vm_rpn_op_B_AND       ; '&'
.db <.vm_rpn_op_ILLEGAL     ; '(single-quote)'
.db <.vm_rpn_op_ILLEGAL     ; '('
.db <.vm_rpn_op_ILLEGAL     ; ')'
.db <.vm_rpn_op_MUL         ; '*'
.db <.vm_rpn_op_ADD         ; '+'
.db <.vm_rpn_op_ILLEGAL     ; ','
.db <.vm_rpn_op_SUB         ; '-'
.db <.vm_rpn_op_ILLEGAL     ; '.'
.db <.vm_rpn_op_DIV         ; '/'

; Skip '0123456789:;<=>?'
.rept 16
    .db <.vm_rpn_op_ILLEGAL
.endm

.db <.vm_rpn_op_ABS         ; '@'
.db <.vm_rpn_op_ASR         ; 'A'
; Skip 'BCDEFGHIJKL'
.rept 10
    .db <.vm_rpn_op_ILLEGAL
.endm
.db <.vm_rpn_op_SHL         ; 'L'
.db <.vm_rpn_op_MAX         ; 'M'
.db <.vm_rpn_op_ILLEGAL
.db <.vm_rpn_op_ILLEGAL
.db <.vm_rpn_op_ILLEGAL
.db <.vm_rpn_op_ILLEGAL
.db <.vm_rpn_op_SHR         ; 'R'
; Skip 'STUVWXYZ[\]'
.rept 11
    .db <.vm_rpn_op_ILLEGAL
.endm
.db <.vm_rpn_op_B_XOR       ; '^'
.db <.vm_rpn_op_ILLEGAL
; Skip '`abcdefghijkl'
.rept 13
    .db <.vm_rpn_op_ILLEGAL
.endm
.db <.vm_rpn_op_MIN         ; 'm'
.db <.vm_rpn_op_ILLEGAL
.db <.vm_rpn_op_ILLEGAL
; Skip 'pqrstuvwxyz{'
.rept 12
    .db <.vm_rpn_op_ILLEGAL
.endm
.db <.vm_rpn_op_B_OR        ; '|'
.db <.vm_rpn_op_ILLEGAL
.db <.vm_rpn_op_B_NOT       ; '~'
.db <.vm_rpn_op_ILLEGAL

.bndry 0x100
.vm_rpn_op_addr_hi:
.db >.vm_rpn_op_STOP        ; R_STOP

; logical operators
.db >.vm_rpn_op_EQ          ; 1
.db >.vm_rpn_op_LT          ; 2
.db >.vm_rpn_op_LE          ; 3
.db >.vm_rpn_op_GT          ; 4
.db >.vm_rpn_op_GE          ; 5
.db >.vm_rpn_op_NE          ; 6
.db >.vm_rpn_op_AND         ; 7
.db >.vm_rpn_op_OR          ; 8
.db >.vm_rpn_op_NOT         ; 9

.db >.vm_rpn_op_ILLEGAL
.db >.vm_rpn_op_ILLEGAL
.db >.vm_rpn_op_ILLEGAL
.db >.vm_rpn_op_ILLEGAL
.db >.vm_rpn_op_ILLEGAL
.db >.vm_rpn_op_ILLEGAL

; Hard-coded SHL/SHR/ASR
.db >.vm_rpn_op_TODO        ; SHL4 (0x10)
.db >.vm_rpn_op_SHL7        ; SHL7 (0x11)
.db >.vm_rpn_op_TODO        ; SHR4 (0x12)
.db >.vm_rpn_op_TODO        ; SHR7 (0x13)
.db >.vm_rpn_op_TODO        ; ASR4 (0x14)
.db >.vm_rpn_op_ASR7        ; ASR7 (0x15)
.db >.vm_rpn_op_CLAMP0      ; CLAMP0 (0x16)

; Skip some more invisible characters
.rept 9
    .db >.vm_rpn_op_ILLEGAL
.endm

.db >.vm_rpn_op_ILLEGAL     ; ' '
.db >.vm_rpn_op_ILLEGAL     ; '!'
.db >.vm_rpn_op_ILLEGAL     ; '"'
.db >.vm_rpn_op_ILLEGAL     ; '#'
.db >.vm_rpn_op_ILLEGAL     ; '$'
.db >.vm_rpn_op_MOD         ; '%'
.db >.vm_rpn_op_B_AND       ; '&'
.db >.vm_rpn_op_ILLEGAL     ; '(single-quote)'
.db >.vm_rpn_op_ILLEGAL     ; '('
.db >.vm_rpn_op_ILLEGAL     ; ')'
.db >.vm_rpn_op_MUL         ; '*'
.db >.vm_rpn_op_ADD         ; '+'
.db >.vm_rpn_op_ILLEGAL     ; ','
.db >.vm_rpn_op_SUB         ; '-'
.db >.vm_rpn_op_ILLEGAL     ; '.'
.db >.vm_rpn_op_DIV         ; '/'

; Skip '0123456789:;<=>?'
.rept 16
    .db >.vm_rpn_op_ILLEGAL
.endm

.db >.vm_rpn_op_ABS         ; '@'
.db >.vm_rpn_op_ASR         ; 'A'
; Skip 'BCDEFGHIJKL'
.rept 10
    .db >.vm_rpn_op_ILLEGAL
.endm
.db >.vm_rpn_op_SHL         ; 'L'
.db >.vm_rpn_op_MAX         ; 'M'
.db >.vm_rpn_op_ILLEGAL
.db >.vm_rpn_op_ILLEGAL
.db >.vm_rpn_op_ILLEGAL
.db >.vm_rpn_op_ILLEGAL
.db >.vm_rpn_op_SHR         ; 'R'
; Skip 'STUVWXYZ[\]'
.rept 11
    .db >.vm_rpn_op_ILLEGAL
.endm
.db >.vm_rpn_op_B_XOR       ; '^'
.db >.vm_rpn_op_ILLEGAL
; Skip '`abcdefghijkl'
.rept 13
    .db >.vm_rpn_op_ILLEGAL
.endm
.db >.vm_rpn_op_MIN         ; 'm'
.db >.vm_rpn_op_ILLEGAL
.db >.vm_rpn_op_ILLEGAL
; Skip 'pqrstuvwxyz{'
.rept 12
    .db >.vm_rpn_op_ILLEGAL
.endm
.db >.vm_rpn_op_B_OR        ; '|'
.db >.vm_rpn_op_ILLEGAL
.db >.vm_rpn_op_B_NOT       ; '~'
.db >.vm_rpn_op_ILLEGAL

.bndry 0x100
.vm_rpn_op_impl:

; .ADD
.vm_rpn_op_ADD:
    clc
    adc .vm_rpn_B_lo,x
    sta .vm_rpn_A_lo,x
    tya
    adc .vm_rpn_B_hi,x
    sta .vm_rpn_A_hi,x
    jmp .vm_rpn_op_next_dec_stack_ptr

; .SUB
.vm_rpn_op_SUB:
    sec
    sbc .vm_rpn_B_lo,x
    sta .vm_rpn_A_lo,x
    tya
    sbc .vm_rpn_B_hi,x
    sta .vm_rpn_A_hi,x
    jmp .vm_rpn_op_next_dec_stack_ptr

; .DIV
.vm_rpn_op_DIV:
    pha
    lda .vm_rpn_B_lo,x
    sta	__divsint_PARM_2
    lda .vm_rpn_B_hi,x
    sta	(__divsint_PARM_2 + 1)
    pla
    tya
    stx *REGTEMP
    tax
    jsr	__divsint
    sta .vm_rpn_A_lo,x
    tya
    ldx *REGTEMP
    sta .vm_rpn_A_hi,x
    jmp .vm_rpn_op_next_dec_stack_ptr

; .MOD
.vm_rpn_op_MOD:
    pha
    lda .vm_rpn_B_lo,x
    sta	__modsint_PARM_2
    lda .vm_rpn_B_hi,x
    sta	(__modsint_PARM_2 + 1)
    pla
    tya
    stx *REGTEMP
    tax
    jsr	__modsint
    sta .vm_rpn_A_lo,x
    tya
    ldx *REGTEMP
    sta .vm_rpn_A_hi,x
    jmp .vm_rpn_op_next_dec_stack_ptr

; .B_AND
.vm_rpn_op_B_AND:
    and .vm_rpn_B_lo,x
    sta .vm_rpn_A_lo,x
    tya
    and .vm_rpn_B_hi,x
    sta .vm_rpn_A_hi,x
    jmp .vm_rpn_op_next_dec_stack_ptr

; .B_OR
.vm_rpn_op_B_OR:
    ora .vm_rpn_B_lo,x
    sta .vm_rpn_A_lo,x
    tya
    ora .vm_rpn_B_hi,x
    sta .vm_rpn_A_hi,x
    jmp .vm_rpn_op_next_dec_stack_ptr

; .B_XOR
.vm_rpn_op_B_XOR:
    eor .vm_rpn_B_lo,x
    sta .vm_rpn_A_lo,x
    tya
    eor .vm_rpn_B_hi,x
    sta .vm_rpn_A_hi,x
    jmp .vm_rpn_op_next_dec_stack_ptr

; .B_NOT
.vm_rpn_op_B_NOT:
    lda .vm_rpn_B_lo,x
    eor #0xFF
    sta .vm_rpn_B_lo,x
    lda .vm_rpn_B_hi,x
    eor #0xFF
    sta .vm_rpn_B_hi,x
    jmp .vm_rpn_op_next

; .ABS
.vm_rpn_op_ABS:
    lda .vm_rpn_B_hi,x
    bpl 1$
    tay
    lda .vm_rpn_B_lo,x
    eor #0xFF
    clc
    adc #1
    sta .vm_rpn_B_lo,x
    tya
    eor #0xFF
    adc #0
    sta .vm_rpn_B_hi,x
1$:
    jmp .vm_rpn_op_next

; .MIN
.vm_rpn_op_MIN:
    sec
    sbc .vm_rpn_B_lo,x
    tya
    sbc .vm_rpn_B_hi,x
    ; A - B > 0 -> needs swap
    bmi 1$
    lda .vm_rpn_B_lo,x
    sta .vm_rpn_A_lo,x
    lda .vm_rpn_B_hi,x
    sta .vm_rpn_A_hi,x
1$:
    jmp .vm_rpn_op_next_dec_stack_ptr

; .MAX
.vm_rpn_op_MAX:
    sec
    sbc .vm_rpn_B_lo,x
    tya
    sbc .vm_rpn_B_hi,x
    ; A - B < 0 -> needs swap
    bpl 1$
    lda .vm_rpn_B_lo,x
    sta .vm_rpn_A_lo,x
    lda .vm_rpn_B_hi,x
    sta .vm_rpn_A_hi,x
1$:
    jmp .vm_rpn_op_next_dec_stack_ptr

.vm_rpn_op_STOP:
    jmp .vm_rpn_end

.vm_rpn_op_ILLEGAL:
    jmp .vm_rpn_op_ILLEGAL

.vm_rpn_op_TODO:
    jmp .vm_rpn_op_TODO

.vm_rpn_op_EQ:
    jsr .vm_rpn_compare_A_B
    beq 1$
    lda #1
1$:
    eor #1
    jmp .vm_rpn_store_a_to_A
.vm_rpn_op_LT:
    jsr .vm_rpn_compare_A_B
    bpl 1$
    beq 1$
    lda #1
    jmp .vm_rpn_store_a_to_A
1$:
    lda #0
    jmp .vm_rpn_store_a_to_A
.vm_rpn_op_LE:
    jsr .vm_rpn_compare_A_B
    bpl 1$
    lda #1
    jmp .vm_rpn_store_a_to_A
1$:
    lda #0
    jmp .vm_rpn_store_a_to_A
.vm_rpn_op_GT:
    jsr .vm_rpn_compare_A_B
    bpl 1$
    lda #0
    jmp .vm_rpn_store_a_to_A
1$:
    lda #1
    jmp .vm_rpn_store_a_to_A
.vm_rpn_op_GE:
    jsr .vm_rpn_compare_A_B
    bpl 1$
    beq 1$
    lda #0
    jmp .vm_rpn_store_a_to_A
1$:
    lda #1
    jmp .vm_rpn_store_a_to_A
.vm_rpn_op_NE:
    jsr .vm_rpn_compare_A_B
    beq 1$
    lda #1
1$:
    jmp .vm_rpn_store_a_to_A

.vm_rpn_op_OR:
    lda .vm_rpn_A_lo,x
    ora .vm_rpn_A_lo,x
    beq 1$
    ; A true -> result true, early-out
    lda #1
    jmp .vm_rpn_store_a_to_A
1$:
    lda .vm_rpn_B_lo,x
    ora .vm_rpn_B_lo,x
    beq 2$
    lda #1
2$:
    jmp .vm_rpn_store_a_to_A

.vm_rpn_op_NOT:
    lda .vm_rpn_B_lo,x
    ora .vm_rpn_B_hi,x
    beq 1$
    lda #1
1$:
    eor #1
    jmp .vm_rpn_op_next

.vm_rpn_op_CLAMP0:
    lda .vm_rpn_B_hi,x
    bpl 1$
    lda #0
    sta .vm_rpn_B_lo,x
    sta .vm_rpn_B_hi,x
1$:
    jmp .vm_rpn_op_next

.vm_rpn_op_SHL7:
    lda .vm_rpn_B_hi,x
    lsr
    lda .vm_rpn_B_lo,x
    ror
    sta .vm_rpn_B_hi,x
    lda #0
    ror
    sta .vm_rpn_B_lo,x
    jmp .vm_rpn_op_next

.vm_rpn_op_ASR7:
    lda .vm_rpn_B_lo,x
    asl
    lda .vm_rpn_B_hi,x
    rol
    sta .vm_rpn_B_lo,x
    lda #0
    bcc 1$
    ; < 0
    lda #0xFF
1$:
    sta .vm_rpn_B_hi,x
    jmp .vm_rpn_op_next

.vm_rpn_op_SHL:
    sty *REGTEMP
    ldy .vm_rpn_B_lo,x
1$:
    asl
    rol *REGTEMP
    dey
    bne 1$
    sta .vm_rpn_A_lo,x
    lda *REGTEMP
    sta .vm_rpn_A_hi,x
    jmp .vm_rpn_op_next_dec_stack_ptr

.vm_rpn_op_SHR:
    sty *REGTEMP
    ldy .vm_rpn_B_lo,x
1$:
    lsr *REGTEMP
    ror
    dey
    bne 1$
    sta .vm_rpn_A_lo,x
    lda *REGTEMP
    sta .vm_rpn_A_hi,x
    jmp .vm_rpn_op_next_dec_stack_ptr

.vm_rpn_op_ASR:
    sta *REGTEMP
    tya
    ldy .vm_rpn_B_lo,x
1$:
    cmp #0x80
    ror
    ror *REGTEMP
    dey
    bne 1$
    sta .vm_rpn_A_hi,x
    lda *REGTEMP
    sta .vm_rpn_A_lo,x
    jmp .vm_rpn_op_next_dec_stack_ptr

.vm_rpn_op_AND:
    ora .vm_rpn_A_hi,x
    beq 1$
    ; A != 0 -> result is bool(B)
    lda .vm_rpn_B_lo,x
    ora .vm_rpn_B_hi,x
    beq 1$  ; bool(B) == 0
    lda #1  ; bool(B) == 1
1$:
    ; A == 0 -> result is 0
.vm_rpn_store_a_to_A:
    sta .vm_rpn_A_lo,x
    lda #0
    sta .vm_rpn_A_hi,x
    jmp .vm_rpn_op_next_dec_stack_ptr

.vm_rpn_op_MUL:
    pha
    lda .vm_rpn_B_lo,x
    sta	__mulint_PARM_2
    lda .vm_rpn_B_hi,x
    sta	(__mulint_PARM_2 + 1)
    pla
    tya
    stx *REGTEMP
    tax
    jsr	__mulint
    sta .vm_rpn_A_lo,x
    tya
    ldx *REGTEMP
    sta .vm_rpn_A_hi,x
    jmp .vm_rpn_op_next_dec_stack_ptr

.vm_rpn_compare_A_B:
    sec
    sbc .vm_rpn_B_lo,x
    tya
    sbc .vm_rpn_B_hi,x
    rts

;
; In:
;      AY: 16-bit index (lo byte in Y, hi byte in A)
;      flags: Must reflect contents of A (hi byte)
; Out: 
;      REGTEMP:
;        - AY >= 0: 16-bit pointer to heap
;        - AY < 0:  16-bit pointer to stack
; 
.vm_rpn_set_op_eval_ptr:
    ; if (idx < 0)
    bmi 1$
    ; A = script_memory + idx;
    sta *REGTEMP+1
    tya
    asl
    clc
    adc #<_script_memory
    sta *REGTEMP
    lda *REGTEMP+1
    adc #>_script_memory
    sta *REGTEMP+1 
    rts
1$:
    ; A = ARGS + idx;
    tya
    asl
    clc
    adc *.ARGS
    sta *REGTEMP
    lda #>(_script_memory+2*VM_HEAP_SIZE)
    sta *REGTEMP+1
    rts

;
; Prefetches all bytes from bytecode bank, and pushes them on stack
;
.area _HOME
.vm_rpn_prefetch:
    ; Save old bank
    ; ...
    ; Load PC and bank
    ldy #0
    lda [*.THIS],y
    iny
    sta *.PC
    lda [*.THIS],y
    iny
    sta *.PC+1
    lda [*.THIS],y
    sta *.bank
    ; bank-switch to bytecode bank
    tay
    sta *__current_bank
    sta .identity,y
    ; Start from end of RPN ops, and push all bytes on stack, preceded by R_STOP
    ldy #0
    lda [*.PC],y    ; Length
    tay
    dey
    sty *.RPN_LENGTH
1$:
    lda [*.PC],y
    pha
    dey
    bne 1$
    ; Switch back to vm_rpn bank
    lda #<___bank_vm_rpn
    tay
    sta *__current_bank
    sta .identity,y
    jmp .vm_rpn_prefetch_done
