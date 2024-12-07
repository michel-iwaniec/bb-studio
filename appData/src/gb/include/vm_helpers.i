; Set to enable profiling of VM calls
MESEN_LUA_PROFILING = 0

.macro STORE_THIS
    sta *.THIS_
    stx *.THIS_+1
.endm

.macro LOAD_STACK_IDX
    ldy #08
    lda [*.THIS_],y
    tax
.endm

.macro STORE_STACK_IDX
    ldy #08
    txa
    sta [*.THIS_],y
.endm

.macro ADD_STACK_IDX val
    lda val
    asl
    ldy #08
    adc [*.THIS_],y
    sta [*.THIS_],y
    tax
.endm

.macro SUB_STACK_IDX val
    lda val
    asl
    eor #0xFF
    sec
    ldy #08
    adc [*.THIS_],y
    sta [*.THIS_],y
    tax
.endm
