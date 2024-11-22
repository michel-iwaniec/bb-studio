    .module     vm_ui_a
    .include    "global.s"

; gbdk-nes: Use _BSS instead of _DATA to fix misaligned _XINIT -> _DATA   .area   _DATA

.area _BSS

__itoa_fmt_len::
    .ds 0x01
;.itoa_fmt_buf::
;    .ds 0x03

    .area   _CODE_255

;.globl b_itoa_fmt
;b_itoa_fmt = 255

;_itoa_fmt::
