                                      1 ;
                                      2 ; crt0.s for NES, using UNROM-512 (mapper30) with single-screen mirroring variant
                                      3 ;
                                      4 ; Provides:
                                      5 ;  * Start-up code clearing RAM and VRAM
                                      6 ;  * Constant-cycle-time NMI handler, performing sprite DMA and VRAM writes via transfer buffer at $100
                                      7 ;  * 16-bit frame counter _sys_time, to support VM routines
                                      8 .module crt0
                                      9 .include    "global.s"
                                      1         ;; Maximum number of times LCD ISR can be repeatedly called
                         00000004     2         .MAX_LCD_ISR_CALLS = 4
                                      3         ;; Total number is +1 to support VBL ISR with the same logic
                         00000005     4         .MAX_DEFERRED_ISR_CALLS = .MAX_LCD_ISR_CALLS+1
                                      5 
                                      6         ;; Transfer buffer (lower half of hardware stack)
                         00000100     7         __vram_transfer_buffer = 0x100
                                      8         ;; Number of 8-cycles available each frame for transfer buffer
                         000000A9     9         VRAM_DELAY_CYCLES_X8  = 169 ; 171
                                     10 
                                     11         ;;  Keypad
                         00000008    12         .UP             = 0x08
                         00000004    13         .DOWN           = 0x04
                         00000002    14         .LEFT           = 0x02
                         00000001    15         .RIGHT          = 0x01
                         00000080    16         .A              = 0x80
                         00000040    17         .B              = 0x40
                         00000020    18         .SELECT         = 0x20
                         00000010    19         .START          = 0x10
                                     20 
                                     21         ;;  Screen dimensions (in tiles)
                         00000020    22         .DEVICE_SCREEN_WIDTH            = 32
                         0000001E    23         .DEVICE_SCREEN_HEIGHT           = 30
                         00000020    24         .DEVICE_SCREEN_BUFFER_WIDTH     = 32
                         0000001E    25         .DEVICE_SCREEN_BUFFER_HEIGHT    = 30
                         0000001F    26         .MAXCURSPOSX    = 31
                         0000001D    27         .MAXCURSPOSY    = 29
                                     28 
                         00000100    29         .SCREENWIDTH    = 256
                         000000F0    30         .SCREENHEIGHT   = 240
                                     31 
                                     32         ;; NAMETABLES
                         00002000    33         PPU_NT0         = 0x2000
                         000023C0    34         PPU_AT0         = 0x23C0
                                     35 
                         00000010    36         ATTRIBUTE_WIDTH                = 16
                         0000000F    37         ATTRIBUTE_HEIGHT               = 15
                         00000008    38         ATTRIBUTE_PACKED_WIDTH         = 8
                         00000008    39         ATTRIBUTE_PACKED_HEIGHT        = 8
                         00000003    40         ATTRIBUTE_MASK_TL = 0b00000011
                         0000000C    41         ATTRIBUTE_MASK_TR = 0b00001100
                         00000030    42         ATTRIBUTE_MASK_BL = 0b00110000
                         000000C0    43         ATTRIBUTE_MASK_BR = 0b11000000
                                     44         
                         00000001    45         GBDK_NES_8X8_ATTRIBUTES = 1
                         00000001    46         GBDK_NES_EVERY_SCANLINE_IRQ = 1
                         00008000    47         CFG_REG = 0x8000
                         0000C000    48         PRG_REG = 0xC000
                         00000001    49         CFG_CHR_A12         = 0b00000001
                         00000002    50         CFG_CHR_A13         = 0b00000010
                         00000004    51         CFG_CHR_A14         = 0b00000100
                         00000008    52         CFG_4S_DISABLE      = 0b00001000
                         00000010    53         CFG_SWP_SPR_4S      = 0b00010000
                         00000020    54         CFG_WRAM_ENABLE     = 0b00100000
                         00000040    55         CFG_WRAM_BANK       = 0b01000000
                         00000080    56         CFG_IRQ_ENABLE      = 0b10000000
                                     57 
                                     58         ;; Hardware registers / masks
                         00002000    59         PPUCTRL             = 0x2000
                         00000080    60         PPUCTRL_NMI         = 0b10000000
                         00000000    61         PPUCTRL_SPR_8X8     = 0b00000000
                         00000020    62         PPUCTRL_SPR_8X16    = 0b00100000
                         00000010    63         PPUCTRL_BG_CHR      = 0b00010000
                         00000008    64         PPUCTRL_SPR_CHR     = 0b00001000
                         00000004    65         PPUCTRL_INC32       = 0b00000100
                                     66         ;
                         00002001    67         PPUMASK             = 0x2001
                         00000080    68         PPUMASK_BLUE        = 0b10000000
                         00000040    69         PPUMASK_RED         = 0b01000000
                         00000020    70         PPUMASK_GREEN       = 0b00100000
                         00000010    71         PPUMASK_SHOW_SPR    = 0b00010000
                         00000008    72         PPUMASK_SHOW_BG     = 0b00001000
                         00000004    73         PPUMASK_SHOW_SPR_LC = 0b00000100
                         00000002    74         PPUMASK_SHOW_BG_LC  = 0b00000010
                         00000001    75         PPUMASK_MONOCHROME  = 0b00000001
                                     76         ;
                         00002002    77         PPUSTATUS       = 0x2002
                                     78         ;
                         00002003    79         OAMADDR         = 0x2003
                                     80         ;
                         00002004    81         OAMDATA         = 0x2004
                                     82         ;
                         00002005    83         PPUSCROLL       = 0x2005
                                     84         ;
                         00002006    85         PPUADDR         = 0x2006
                                     86         ;
                         00002007    87         PPUDATA         = 0x2007
                                     88         ;
                         00004014    89         OAMDMA          = 0x4014
                                     90         ;
                         00004016    91         JOY1            = 0x4016
                                     92         ;
                         00004017    93         JOY2            = 0x4017
                                     94 
                                     95         ;; OAM related constants
                                     96 
                         00000040    97         OAM_COUNT       = 64  ; number of OAM entries in OAM RAM
                                     98 
                         00000000    99         OAM_POS_Y       = 0
                         00000001   100         OAM_TILE_INDEX  = 1
                         00000002   101         OAM_ATTRIBUTES  = 2
                         00000003   102         OAM_POS_X       = 3
                                    103 
                         00000020   104         OAMF_PRI        = 0b00100000 ; Priority
                         00000080   105         OAMF_YFLIP      = 0b10000000 ; Y flip
                         00000040   106         OAMF_XFLIP      = 0b01000000 ; X flip
                                    107 
                                    108         ;; GBDK library screen modes
                                    109 
                         00000001   110         .G_MODE         = 0x01  ; Graphic mode
                         00000002   111         .T_MODE         = 0x02  ; Text mode (bit 2)
                         00000002   112         .T_MODE_OUT     = 0x02  ; Text mode output only
                         00000003   113         .T_MODE_INOUT   = 0x03  ; Text mode with input
                         00000004   114         .M_NO_SCROLL    = 0x04  ; Disables scrolling of the screen in text mode
                         00000008   115         .M_NO_INTERP    = 0x08  ; Disables special character interpretation
                                    116 
                                    117         ;; Table of routines for modes
                         0000FFE0   118         .MODE_TABLE     = 0xFFE0
                                    119 
                                    120         ;; C related
                                    121         ;; Overheap of a banked call.  Used for parameters
                                    122         ;;  = ret + real ret + bank
                                    123 
                         00000006   124         .BANKOV         = 6
                                    125 
                                    126         .globl  __current_bank
                                    127         .globl  __shadow_OAM_base
                                    128 
                                    129         ;; Global variables
                                    130         .globl  .mode
                                    131         .define .tmp "REGTEMP"
                                    132 
                                    133         .globl _shadow_PPUCTRL, _shadow_PPUMASK
                                    134         .globl _bkg_scroll_x, _bkg_scroll_y
                                    135         .globl __crt0_paletteShadow
                                    136         .globl _attribute_shadow, _attribute_row_dirty
                                    137         .globl _attribute_shadow_offset
                                    138         
                                    139         ;; Identity table for register-to-register-adds and bankswitching
                                    140         .globl .identity, _identity
                                    141 
                                    142         ;; Global routines
                                    143 
                                    144         .globl  .display_off, .display_on
                                    145         .globl  .wait_vbl_done
                                    146 
                                    147         ;; Symbols defined at link time
                                    148         .globl _shadow_OAM, __vram_transfer_buffer
                                    149 
                                    150         ;; Main user routine
                                    151         .globl  _main
                                    152 
                                    153 .macro DIV_PART divident divisor ?lbl
                                    154         rol divident
                                    155         rol
                                    156         sec
                                    157         sbc divisor
                                    158         bcs lbl
                                    159         adc divisor
                                    160 lbl:
                                    161 .endm
                                    162 .macro FAST_DIV8 divident divisor
                                    163         ; returns quotient in A
                                    164         .rept 8
                                    165                 DIV_PART divident divisor
                                    166         .endm
                                    167         lda divident
                                    168         eor #0xFF
                                    169 .endm
                                    170 .macro FAST_MOD8 divident divisor
                                    171         ; returns modulus in A
                                    172         .rept 8
                                    173                 DIV_PART divident divisor
                                    174         .endm
                                    175 .endm
                                     10 
                                     11 ; OAM CPU page
                         00000200    12 _shadow_OAM             = 0x200
                                     13 ; Attribute shadow (64 bytes, leaving 56 bytes available for CPU stack)
                         00007F00    14 _attribute_shadow       = 0x7F00 ;0x188
                                     15 
                                     16 .macro WRITE_PALETTE_SHADOW
                                     17     lda #>0x3F00
                                     18     sta PPUADDR
                                     19     lda #<0x3F00
                                     20     sta PPUADDR
                                     21     ldx __crt0_paletteShadow
                                     22     i = 0
                                     23 .rept 8
                                     24     stx PPUDATA
                                     25     lda (__crt0_paletteShadow+1+3*i+0)
                                     26     sta PPUDATA
                                     27     lda (__crt0_paletteShadow+1+3*i+1)
                                     28     sta PPUDATA
                                     29     lda (__crt0_paletteShadow+1+3*i+2)
                                     30     sta PPUDATA
                                     31     i = i + 1
                                     32 .endm
                                     33 .endm
                                     34 
                                     35        ;; ****************************************
                                     36 
                                     37         ;; Ordering of segments for the linker
                                     38         ;; Code that really needs to be in the fixed bank
                                     39         .area _CODE
                                     40         .area _HOME
                                     41         ;; Similar to _HOME
                                     42         .area _BASE
                                     43         ;; Constant data
                                     44         .area _LIT
                                     45         .area _RODATA
                                     46         ;; Constant data, used to init _DATA
                                     47         .area _INITIALIZER
                                     48         .area _XINIT
                                     49         ;; Code, used to init _DATA
                                     50         .area _GSINIT 
                                     51         .area _GSFINAL
                                     52         ;; Uninitialised ram data
                                     53         .area _DATA
                                     54         .area _BSS
                                     55         ;; Initialised in ram data
                                     56         .area _INITIALIZED
                                     57         ;; For malloc
                                     58         .area _HEAP
                                     59         .area _HEAP_END
                                     60 
                                     61 .area	OSEG (PAG, OVR)
                                     62 .area	GBDKOVR (PAG, OVR)
                                     63 .area _ZP (PAG)
    00000020                         64 __current_bank::                        .ds 1
    00000021                         65 _sys_time::                             .ds 2
    00000023                         66 _sys_time_old::                         .ds 1
    00000024                         67 _shadow_PPUCTRL::                       .ds 1
    00000025                         68 _shadow_PPUMASK::                       .ds 1
    00000026                         69 __crt0_spritePageValid::                .ds 1
    00000027                         70 __crt0_disableNMI:                      .ds 1
    00000028                         71 _bkg_scroll_x::                         .ds 1
    00000029                         72 _bkg_scroll_y::                         .ds 1
    0000002A                         73 _attribute_row_dirty::                  .ds 1
    0000002B                         74 _attribute_column_dirty::               .ds 1
    0000002C                         75 _attribute_shadow_offset::              .ds 1
    0000002D                         76 .crt0_forced_blanking::                 .ds 1
    0000002E                         77 __SYSTEM::                              .ds 1
    0000002F                         78 .ldx_mapper_config:                     .ds 1
    00000030                         79 __vbl_isr_mapper_config:                .ds 1
    00000031                         80 __ESI_scanline_counter:                 .ds 1
    00000032                         81 __ESI_write_index:                      .ds 1
    00000033                         82 __CFG_REG_cache:                        .ds 1
                                     83 
                                     84 .define __crt0_NMITEMP "___SDCC_m6502_ret4"
                                     85 
                                     86 .area _BSS
    00006013                         87 __crt0_paletteShadow::                  .ds 25
    0000602C                         88 __shadow_OAM_base::                     .ds 1
    0000602D                         89 .mode::                                 .ds 1
    0000602E                         90 __lcd_isr_PPUCTRL:                      .ds (2*.MAX_DEFERRED_ISR_CALLS)
    00006038                         91 __lcd_isr_PPUMASK:                      .ds (2*.MAX_DEFERRED_ISR_CALLS)
    00006042                         92 __lcd_isr_scroll_x:                     .ds (2*.MAX_DEFERRED_ISR_CALLS)
    0000604C                         93 __lcd_isr_scroll_y:                     .ds (2*.MAX_DEFERRED_ISR_CALLS)
    00006056                         94 __lcd_isr_delay_num_scanlines:          .ds (2*.MAX_DEFERRED_ISR_CALLS)
    00006060                         95 __lcd_isr_mapper_config:                .ds (2*.MAX_DEFERRED_ISR_CALLS)
    0000606A                         96 __lcd_isr_ppuaddr_lo:                   .ds (2*.MAX_DEFERRED_ISR_CALLS)
    00006074                         97 __lcd_isr_num_calls:                    .ds 2
    00006076                         98 __lcd_isr_buf_length:                   .ds 1
    00006077                         99 _attribute_row_dirty_planes::           .ds 4
    0000607B                        100 _attribute_column_dirty_planes::        .ds 4
    0000607F                        101 __lcd_isr_read_buf:                     .ds 1
                                    102 
                                    103 
                                    104 .area _CODE
                                    105 
    0000C000                        106 .bndry 0x100
    0000C000                        107 .identity::
    0000C000                        108 _identity::
                         00000000   109 i = 0
                                    110 .rept 256
                                    111 .db i
                                    112 i = i + 1
                                    113 .endm
    0000C000 00                       1 .db i
                         00000001     2 i = i + 1
    0000C001 01                       1 .db i
                         00000002     2 i = i + 1
    0000C002 02                       1 .db i
                         00000003     2 i = i + 1
    0000C003 03                       1 .db i
                         00000004     2 i = i + 1
    0000C004 04                       1 .db i
                         00000005     2 i = i + 1
    0000C005 05                       1 .db i
                         00000006     2 i = i + 1
    0000C006 06                       1 .db i
                         00000007     2 i = i + 1
    0000C007 07                       1 .db i
                         00000008     2 i = i + 1
    0000C008 08                       1 .db i
                         00000009     2 i = i + 1
    0000C009 09                       1 .db i
                         0000000A     2 i = i + 1
    0000C00A 0A                       1 .db i
                         0000000B     2 i = i + 1
    0000C00B 0B                       1 .db i
                         0000000C     2 i = i + 1
    0000C00C 0C                       1 .db i
                         0000000D     2 i = i + 1
    0000C00D 0D                       1 .db i
                         0000000E     2 i = i + 1
    0000C00E 0E                       1 .db i
                         0000000F     2 i = i + 1
    0000C00F 0F                       1 .db i
                         00000010     2 i = i + 1
    0000C010 10                       1 .db i
                         00000011     2 i = i + 1
    0000C011 11                       1 .db i
                         00000012     2 i = i + 1
    0000C012 12                       1 .db i
                         00000013     2 i = i + 1
    0000C013 13                       1 .db i
                         00000014     2 i = i + 1
    0000C014 14                       1 .db i
                         00000015     2 i = i + 1
    0000C015 15                       1 .db i
                         00000016     2 i = i + 1
    0000C016 16                       1 .db i
                         00000017     2 i = i + 1
    0000C017 17                       1 .db i
                         00000018     2 i = i + 1
    0000C018 18                       1 .db i
                         00000019     2 i = i + 1
    0000C019 19                       1 .db i
                         0000001A     2 i = i + 1
    0000C01A 1A                       1 .db i
                         0000001B     2 i = i + 1
    0000C01B 1B                       1 .db i
                         0000001C     2 i = i + 1
    0000C01C 1C                       1 .db i
                         0000001D     2 i = i + 1
    0000C01D 1D                       1 .db i
                         0000001E     2 i = i + 1
    0000C01E 1E                       1 .db i
                         0000001F     2 i = i + 1
    0000C01F 1F                       1 .db i
                         00000020     2 i = i + 1
    0000C020 20                       1 .db i
                         00000021     2 i = i + 1
    0000C021 21                       1 .db i
                         00000022     2 i = i + 1
    0000C022 22                       1 .db i
                         00000023     2 i = i + 1
    0000C023 23                       1 .db i
                         00000024     2 i = i + 1
    0000C024 24                       1 .db i
                         00000025     2 i = i + 1
    0000C025 25                       1 .db i
                         00000026     2 i = i + 1
    0000C026 26                       1 .db i
                         00000027     2 i = i + 1
    0000C027 27                       1 .db i
                         00000028     2 i = i + 1
    0000C028 28                       1 .db i
                         00000029     2 i = i + 1
    0000C029 29                       1 .db i
                         0000002A     2 i = i + 1
    0000C02A 2A                       1 .db i
                         0000002B     2 i = i + 1
    0000C02B 2B                       1 .db i
                         0000002C     2 i = i + 1
    0000C02C 2C                       1 .db i
                         0000002D     2 i = i + 1
    0000C02D 2D                       1 .db i
                         0000002E     2 i = i + 1
    0000C02E 2E                       1 .db i
                         0000002F     2 i = i + 1
    0000C02F 2F                       1 .db i
                         00000030     2 i = i + 1
    0000C030 30                       1 .db i
                         00000031     2 i = i + 1
    0000C031 31                       1 .db i
                         00000032     2 i = i + 1
    0000C032 32                       1 .db i
                         00000033     2 i = i + 1
    0000C033 33                       1 .db i
                         00000034     2 i = i + 1
    0000C034 34                       1 .db i
                         00000035     2 i = i + 1
    0000C035 35                       1 .db i
                         00000036     2 i = i + 1
    0000C036 36                       1 .db i
                         00000037     2 i = i + 1
    0000C037 37                       1 .db i
                         00000038     2 i = i + 1
    0000C038 38                       1 .db i
                         00000039     2 i = i + 1
    0000C039 39                       1 .db i
                         0000003A     2 i = i + 1
    0000C03A 3A                       1 .db i
                         0000003B     2 i = i + 1
    0000C03B 3B                       1 .db i
                         0000003C     2 i = i + 1
    0000C03C 3C                       1 .db i
                         0000003D     2 i = i + 1
    0000C03D 3D                       1 .db i
                         0000003E     2 i = i + 1
    0000C03E 3E                       1 .db i
                         0000003F     2 i = i + 1
    0000C03F 3F                       1 .db i
                         00000040     2 i = i + 1
    0000C040 40                       1 .db i
                         00000041     2 i = i + 1
    0000C041 41                       1 .db i
                         00000042     2 i = i + 1
    0000C042 42                       1 .db i
                         00000043     2 i = i + 1
    0000C043 43                       1 .db i
                         00000044     2 i = i + 1
    0000C044 44                       1 .db i
                         00000045     2 i = i + 1
    0000C045 45                       1 .db i
                         00000046     2 i = i + 1
    0000C046 46                       1 .db i
                         00000047     2 i = i + 1
    0000C047 47                       1 .db i
                         00000048     2 i = i + 1
    0000C048 48                       1 .db i
                         00000049     2 i = i + 1
    0000C049 49                       1 .db i
                         0000004A     2 i = i + 1
    0000C04A 4A                       1 .db i
                         0000004B     2 i = i + 1
    0000C04B 4B                       1 .db i
                         0000004C     2 i = i + 1
    0000C04C 4C                       1 .db i
                         0000004D     2 i = i + 1
    0000C04D 4D                       1 .db i
                         0000004E     2 i = i + 1
    0000C04E 4E                       1 .db i
                         0000004F     2 i = i + 1
    0000C04F 4F                       1 .db i
                         00000050     2 i = i + 1
    0000C050 50                       1 .db i
                         00000051     2 i = i + 1
    0000C051 51                       1 .db i
                         00000052     2 i = i + 1
    0000C052 52                       1 .db i
                         00000053     2 i = i + 1
    0000C053 53                       1 .db i
                         00000054     2 i = i + 1
    0000C054 54                       1 .db i
                         00000055     2 i = i + 1
    0000C055 55                       1 .db i
                         00000056     2 i = i + 1
    0000C056 56                       1 .db i
                         00000057     2 i = i + 1
    0000C057 57                       1 .db i
                         00000058     2 i = i + 1
    0000C058 58                       1 .db i
                         00000059     2 i = i + 1
    0000C059 59                       1 .db i
                         0000005A     2 i = i + 1
    0000C05A 5A                       1 .db i
                         0000005B     2 i = i + 1
    0000C05B 5B                       1 .db i
                         0000005C     2 i = i + 1
    0000C05C 5C                       1 .db i
                         0000005D     2 i = i + 1
    0000C05D 5D                       1 .db i
                         0000005E     2 i = i + 1
    0000C05E 5E                       1 .db i
                         0000005F     2 i = i + 1
    0000C05F 5F                       1 .db i
                         00000060     2 i = i + 1
    0000C060 60                       1 .db i
                         00000061     2 i = i + 1
    0000C061 61                       1 .db i
                         00000062     2 i = i + 1
    0000C062 62                       1 .db i
                         00000063     2 i = i + 1
    0000C063 63                       1 .db i
                         00000064     2 i = i + 1
    0000C064 64                       1 .db i
                         00000065     2 i = i + 1
    0000C065 65                       1 .db i
                         00000066     2 i = i + 1
    0000C066 66                       1 .db i
                         00000067     2 i = i + 1
    0000C067 67                       1 .db i
                         00000068     2 i = i + 1
    0000C068 68                       1 .db i
                         00000069     2 i = i + 1
    0000C069 69                       1 .db i
                         0000006A     2 i = i + 1
    0000C06A 6A                       1 .db i
                         0000006B     2 i = i + 1
    0000C06B 6B                       1 .db i
                         0000006C     2 i = i + 1
    0000C06C 6C                       1 .db i
                         0000006D     2 i = i + 1
    0000C06D 6D                       1 .db i
                         0000006E     2 i = i + 1
    0000C06E 6E                       1 .db i
                         0000006F     2 i = i + 1
    0000C06F 6F                       1 .db i
                         00000070     2 i = i + 1
    0000C070 70                       1 .db i
                         00000071     2 i = i + 1
    0000C071 71                       1 .db i
                         00000072     2 i = i + 1
    0000C072 72                       1 .db i
                         00000073     2 i = i + 1
    0000C073 73                       1 .db i
                         00000074     2 i = i + 1
    0000C074 74                       1 .db i
                         00000075     2 i = i + 1
    0000C075 75                       1 .db i
                         00000076     2 i = i + 1
    0000C076 76                       1 .db i
                         00000077     2 i = i + 1
    0000C077 77                       1 .db i
                         00000078     2 i = i + 1
    0000C078 78                       1 .db i
                         00000079     2 i = i + 1
    0000C079 79                       1 .db i
                         0000007A     2 i = i + 1
    0000C07A 7A                       1 .db i
                         0000007B     2 i = i + 1
    0000C07B 7B                       1 .db i
                         0000007C     2 i = i + 1
    0000C07C 7C                       1 .db i
                         0000007D     2 i = i + 1
    0000C07D 7D                       1 .db i
                         0000007E     2 i = i + 1
    0000C07E 7E                       1 .db i
                         0000007F     2 i = i + 1
    0000C07F 7F                       1 .db i
                         00000080     2 i = i + 1
    0000C080 80                       1 .db i
                         00000081     2 i = i + 1
    0000C081 81                       1 .db i
                         00000082     2 i = i + 1
    0000C082 82                       1 .db i
                         00000083     2 i = i + 1
    0000C083 83                       1 .db i
                         00000084     2 i = i + 1
    0000C084 84                       1 .db i
                         00000085     2 i = i + 1
    0000C085 85                       1 .db i
                         00000086     2 i = i + 1
    0000C086 86                       1 .db i
                         00000087     2 i = i + 1
    0000C087 87                       1 .db i
                         00000088     2 i = i + 1
    0000C088 88                       1 .db i
                         00000089     2 i = i + 1
    0000C089 89                       1 .db i
                         0000008A     2 i = i + 1
    0000C08A 8A                       1 .db i
                         0000008B     2 i = i + 1
    0000C08B 8B                       1 .db i
                         0000008C     2 i = i + 1
    0000C08C 8C                       1 .db i
                         0000008D     2 i = i + 1
    0000C08D 8D                       1 .db i
                         0000008E     2 i = i + 1
    0000C08E 8E                       1 .db i
                         0000008F     2 i = i + 1
    0000C08F 8F                       1 .db i
                         00000090     2 i = i + 1
    0000C090 90                       1 .db i
                         00000091     2 i = i + 1
    0000C091 91                       1 .db i
                         00000092     2 i = i + 1
    0000C092 92                       1 .db i
                         00000093     2 i = i + 1
    0000C093 93                       1 .db i
                         00000094     2 i = i + 1
    0000C094 94                       1 .db i
                         00000095     2 i = i + 1
    0000C095 95                       1 .db i
                         00000096     2 i = i + 1
    0000C096 96                       1 .db i
                         00000097     2 i = i + 1
    0000C097 97                       1 .db i
                         00000098     2 i = i + 1
    0000C098 98                       1 .db i
                         00000099     2 i = i + 1
    0000C099 99                       1 .db i
                         0000009A     2 i = i + 1
    0000C09A 9A                       1 .db i
                         0000009B     2 i = i + 1
    0000C09B 9B                       1 .db i
                         0000009C     2 i = i + 1
    0000C09C 9C                       1 .db i
                         0000009D     2 i = i + 1
    0000C09D 9D                       1 .db i
                         0000009E     2 i = i + 1
    0000C09E 9E                       1 .db i
                         0000009F     2 i = i + 1
    0000C09F 9F                       1 .db i
                         000000A0     2 i = i + 1
    0000C0A0 A0                       1 .db i
                         000000A1     2 i = i + 1
    0000C0A1 A1                       1 .db i
                         000000A2     2 i = i + 1
    0000C0A2 A2                       1 .db i
                         000000A3     2 i = i + 1
    0000C0A3 A3                       1 .db i
                         000000A4     2 i = i + 1
    0000C0A4 A4                       1 .db i
                         000000A5     2 i = i + 1
    0000C0A5 A5                       1 .db i
                         000000A6     2 i = i + 1
    0000C0A6 A6                       1 .db i
                         000000A7     2 i = i + 1
    0000C0A7 A7                       1 .db i
                         000000A8     2 i = i + 1
    0000C0A8 A8                       1 .db i
                         000000A9     2 i = i + 1
    0000C0A9 A9                       1 .db i
                         000000AA     2 i = i + 1
    0000C0AA AA                       1 .db i
                         000000AB     2 i = i + 1
    0000C0AB AB                       1 .db i
                         000000AC     2 i = i + 1
    0000C0AC AC                       1 .db i
                         000000AD     2 i = i + 1
    0000C0AD AD                       1 .db i
                         000000AE     2 i = i + 1
    0000C0AE AE                       1 .db i
                         000000AF     2 i = i + 1
    0000C0AF AF                       1 .db i
                         000000B0     2 i = i + 1
    0000C0B0 B0                       1 .db i
                         000000B1     2 i = i + 1
    0000C0B1 B1                       1 .db i
                         000000B2     2 i = i + 1
    0000C0B2 B2                       1 .db i
                         000000B3     2 i = i + 1
    0000C0B3 B3                       1 .db i
                         000000B4     2 i = i + 1
    0000C0B4 B4                       1 .db i
                         000000B5     2 i = i + 1
    0000C0B5 B5                       1 .db i
                         000000B6     2 i = i + 1
    0000C0B6 B6                       1 .db i
                         000000B7     2 i = i + 1
    0000C0B7 B7                       1 .db i
                         000000B8     2 i = i + 1
    0000C0B8 B8                       1 .db i
                         000000B9     2 i = i + 1
    0000C0B9 B9                       1 .db i
                         000000BA     2 i = i + 1
    0000C0BA BA                       1 .db i
                         000000BB     2 i = i + 1
    0000C0BB BB                       1 .db i
                         000000BC     2 i = i + 1
    0000C0BC BC                       1 .db i
                         000000BD     2 i = i + 1
    0000C0BD BD                       1 .db i
                         000000BE     2 i = i + 1
    0000C0BE BE                       1 .db i
                         000000BF     2 i = i + 1
    0000C0BF BF                       1 .db i
                         000000C0     2 i = i + 1
    0000C0C0 C0                       1 .db i
                         000000C1     2 i = i + 1
    0000C0C1 C1                       1 .db i
                         000000C2     2 i = i + 1
    0000C0C2 C2                       1 .db i
                         000000C3     2 i = i + 1
    0000C0C3 C3                       1 .db i
                         000000C4     2 i = i + 1
    0000C0C4 C4                       1 .db i
                         000000C5     2 i = i + 1
    0000C0C5 C5                       1 .db i
                         000000C6     2 i = i + 1
    0000C0C6 C6                       1 .db i
                         000000C7     2 i = i + 1
    0000C0C7 C7                       1 .db i
                         000000C8     2 i = i + 1
    0000C0C8 C8                       1 .db i
                         000000C9     2 i = i + 1
    0000C0C9 C9                       1 .db i
                         000000CA     2 i = i + 1
    0000C0CA CA                       1 .db i
                         000000CB     2 i = i + 1
    0000C0CB CB                       1 .db i
                         000000CC     2 i = i + 1
    0000C0CC CC                       1 .db i
                         000000CD     2 i = i + 1
    0000C0CD CD                       1 .db i
                         000000CE     2 i = i + 1
    0000C0CE CE                       1 .db i
                         000000CF     2 i = i + 1
    0000C0CF CF                       1 .db i
                         000000D0     2 i = i + 1
    0000C0D0 D0                       1 .db i
                         000000D1     2 i = i + 1
    0000C0D1 D1                       1 .db i
                         000000D2     2 i = i + 1
    0000C0D2 D2                       1 .db i
                         000000D3     2 i = i + 1
    0000C0D3 D3                       1 .db i
                         000000D4     2 i = i + 1
    0000C0D4 D4                       1 .db i
                         000000D5     2 i = i + 1
    0000C0D5 D5                       1 .db i
                         000000D6     2 i = i + 1
    0000C0D6 D6                       1 .db i
                         000000D7     2 i = i + 1
    0000C0D7 D7                       1 .db i
                         000000D8     2 i = i + 1
    0000C0D8 D8                       1 .db i
                         000000D9     2 i = i + 1
    0000C0D9 D9                       1 .db i
                         000000DA     2 i = i + 1
    0000C0DA DA                       1 .db i
                         000000DB     2 i = i + 1
    0000C0DB DB                       1 .db i
                         000000DC     2 i = i + 1
    0000C0DC DC                       1 .db i
                         000000DD     2 i = i + 1
    0000C0DD DD                       1 .db i
                         000000DE     2 i = i + 1
    0000C0DE DE                       1 .db i
                         000000DF     2 i = i + 1
    0000C0DF DF                       1 .db i
                         000000E0     2 i = i + 1
    0000C0E0 E0                       1 .db i
                         000000E1     2 i = i + 1
    0000C0E1 E1                       1 .db i
                         000000E2     2 i = i + 1
    0000C0E2 E2                       1 .db i
                         000000E3     2 i = i + 1
    0000C0E3 E3                       1 .db i
                         000000E4     2 i = i + 1
    0000C0E4 E4                       1 .db i
                         000000E5     2 i = i + 1
    0000C0E5 E5                       1 .db i
                         000000E6     2 i = i + 1
    0000C0E6 E6                       1 .db i
                         000000E7     2 i = i + 1
    0000C0E7 E7                       1 .db i
                         000000E8     2 i = i + 1
    0000C0E8 E8                       1 .db i
                         000000E9     2 i = i + 1
    0000C0E9 E9                       1 .db i
                         000000EA     2 i = i + 1
    0000C0EA EA                       1 .db i
                         000000EB     2 i = i + 1
    0000C0EB EB                       1 .db i
                         000000EC     2 i = i + 1
    0000C0EC EC                       1 .db i
                         000000ED     2 i = i + 1
    0000C0ED ED                       1 .db i
                         000000EE     2 i = i + 1
    0000C0EE EE                       1 .db i
                         000000EF     2 i = i + 1
    0000C0EF EF                       1 .db i
                         000000F0     2 i = i + 1
    0000C0F0 F0                       1 .db i
                         000000F1     2 i = i + 1
    0000C0F1 F1                       1 .db i
                         000000F2     2 i = i + 1
    0000C0F2 F2                       1 .db i
                         000000F3     2 i = i + 1
    0000C0F3 F3                       1 .db i
                         000000F4     2 i = i + 1
    0000C0F4 F4                       1 .db i
                         000000F5     2 i = i + 1
    0000C0F5 F5                       1 .db i
                         000000F6     2 i = i + 1
    0000C0F6 F6                       1 .db i
                         000000F7     2 i = i + 1
    0000C0F7 F7                       1 .db i
                         000000F8     2 i = i + 1
    0000C0F8 F8                       1 .db i
                         000000F9     2 i = i + 1
    0000C0F9 F9                       1 .db i
                         000000FA     2 i = i + 1
    0000C0FA FA                       1 .db i
                         000000FB     2 i = i + 1
    0000C0FB FB                       1 .db i
                         000000FC     2 i = i + 1
    0000C0FC FC                       1 .db i
                         000000FD     2 i = i + 1
    0000C0FD FD                       1 .db i
                         000000FE     2 i = i + 1
    0000C0FE FE                       1 .db i
                         000000FF     2 i = i + 1
    0000C0FF FF                       1 .db i
                         00000100     2 i = i + 1
                                    114 
                                    115 .define ProcessDrawList_tempX "__crt0_NMITEMP+2"
                                    116 .define ProcessDrawList_addr  "__crt0_NMITEMP+0"
                                    117 
    0000C100                        118 .bndry 0x100
    0000C100 EA               [ 2]  119     nop         ; Pad to offset, to support zero-terminator value
    0000C101                        120 ProcessDrawList_UnrolledCopyLoop:
                                    121 .rept 32
                                    122 pla             ; +4
                                    123 sta PPUDATA     ; +4
                                    124 .endm
    0000C101 68               [ 4]    1 pla             ; +4
    0000C102 8D 07 20         [ 4]    2 sta PPUDATA     ; +4
    0000C105 68               [ 4]    1 pla             ; +4
    0000C106 8D 07 20         [ 4]    2 sta PPUDATA     ; +4
    0000C109 68               [ 4]    1 pla             ; +4
    0000C10A 8D 07 20         [ 4]    2 sta PPUDATA     ; +4
    0000C10D 68               [ 4]    1 pla             ; +4
    0000C10E 8D 07 20         [ 4]    2 sta PPUDATA     ; +4
    0000C111 68               [ 4]    1 pla             ; +4
    0000C112 8D 07 20         [ 4]    2 sta PPUDATA     ; +4
    0000C115 68               [ 4]    1 pla             ; +4
    0000C116 8D 07 20         [ 4]    2 sta PPUDATA     ; +4
    0000C119 68               [ 4]    1 pla             ; +4
    0000C11A 8D 07 20         [ 4]    2 sta PPUDATA     ; +4
    0000C11D 68               [ 4]    1 pla             ; +4
    0000C11E 8D 07 20         [ 4]    2 sta PPUDATA     ; +4
    0000C121 68               [ 4]    1 pla             ; +4
    0000C122 8D 07 20         [ 4]    2 sta PPUDATA     ; +4
    0000C125 68               [ 4]    1 pla             ; +4
    0000C126 8D 07 20         [ 4]    2 sta PPUDATA     ; +4
    0000C129 68               [ 4]    1 pla             ; +4
    0000C12A 8D 07 20         [ 4]    2 sta PPUDATA     ; +4
    0000C12D 68               [ 4]    1 pla             ; +4
    0000C12E 8D 07 20         [ 4]    2 sta PPUDATA     ; +4
    0000C131 68               [ 4]    1 pla             ; +4
    0000C132 8D 07 20         [ 4]    2 sta PPUDATA     ; +4
    0000C135 68               [ 4]    1 pla             ; +4
    0000C136 8D 07 20         [ 4]    2 sta PPUDATA     ; +4
    0000C139 68               [ 4]    1 pla             ; +4
    0000C13A 8D 07 20         [ 4]    2 sta PPUDATA     ; +4
    0000C13D 68               [ 4]    1 pla             ; +4
    0000C13E 8D 07 20         [ 4]    2 sta PPUDATA     ; +4
    0000C141 68               [ 4]    1 pla             ; +4
    0000C142 8D 07 20         [ 4]    2 sta PPUDATA     ; +4
    0000C145 68               [ 4]    1 pla             ; +4
    0000C146 8D 07 20         [ 4]    2 sta PPUDATA     ; +4
    0000C149 68               [ 4]    1 pla             ; +4
    0000C14A 8D 07 20         [ 4]    2 sta PPUDATA     ; +4
    0000C14D 68               [ 4]    1 pla             ; +4
    0000C14E 8D 07 20         [ 4]    2 sta PPUDATA     ; +4
    0000C151 68               [ 4]    1 pla             ; +4
    0000C152 8D 07 20         [ 4]    2 sta PPUDATA     ; +4
    0000C155 68               [ 4]    1 pla             ; +4
    0000C156 8D 07 20         [ 4]    2 sta PPUDATA     ; +4
    0000C159 68               [ 4]    1 pla             ; +4
    0000C15A 8D 07 20         [ 4]    2 sta PPUDATA     ; +4
    0000C15D 68               [ 4]    1 pla             ; +4
    0000C15E 8D 07 20         [ 4]    2 sta PPUDATA     ; +4
    0000C161 68               [ 4]    1 pla             ; +4
    0000C162 8D 07 20         [ 4]    2 sta PPUDATA     ; +4
    0000C165 68               [ 4]    1 pla             ; +4
    0000C166 8D 07 20         [ 4]    2 sta PPUDATA     ; +4
    0000C169 68               [ 4]    1 pla             ; +4
    0000C16A 8D 07 20         [ 4]    2 sta PPUDATA     ; +4
    0000C16D 68               [ 4]    1 pla             ; +4
    0000C16E 8D 07 20         [ 4]    2 sta PPUDATA     ; +4
    0000C171 68               [ 4]    1 pla             ; +4
    0000C172 8D 07 20         [ 4]    2 sta PPUDATA     ; +4
    0000C175 68               [ 4]    1 pla             ; +4
    0000C176 8D 07 20         [ 4]    2 sta PPUDATA     ; +4
    0000C179 68               [ 4]    1 pla             ; +4
    0000C17A 8D 07 20         [ 4]    2 sta PPUDATA     ; +4
    0000C17D 68               [ 4]    1 pla             ; +4
    0000C17E 8D 07 20         [ 4]    2 sta PPUDATA     ; +4
    0000C181                        125 ProcessDrawList_DoOneTransfer:
    0000C181 68               [ 4]  126     pla                                         ; +4
    0000C182 F0 2B            [ 4]  127     beq ProcessDrawList_EndOfList               ; +2/3
    0000C184 85 49            [ 3]  128     sta *ProcessDrawList_addr                   ; +3
    0000C186 68               [ 4]  129     pla                                         ; +4
    0000C187 8D 00 20         [ 4]  130     sta PPUCTRL                                 ; +4
    0000C18A 68               [ 4]  131     pla                                         ; +4
    0000C18B 8D 06 20         [ 4]  132     sta PPUADDR                                 ; +4
                                    133     
                                    134     ;tay
                                    135     ;sta [*.identity_ptr],y
                                    136     
    0000C18E 68               [ 4]  137     pla                                         ; +4
    0000C18F 8D 06 20         [ 4]  138     sta PPUADDR                                 ; +4
                                    139 
    0000C192 68               [ 4]  140     pla
    0000C193 8D 00 80         [ 4]  141     sta CFG_REG ;0xC000
                                    142 
    0000C196 EA               [ 2]  143     nop                                         ; +2
    0000C197 6C 49 00         [ 5]  144     jmp [ProcessDrawList_addr]                  ; +5
                                    145     ; Total = 4 + 2 + 2 + 4 + 3 + 6*4 + 2 + 2 + 5 = 48 for each transfer (...+ 8*NumBytesCopied)
                                    146     ;         4 + 3 + 14 = 7 + 14 = 21 fixed-cost exit
                                    147 
                                    148 ; .bndry 0x100 (skip alignment as previous alignment means page-cross won't happen)
    0000C19A                        149 __crt0_doSpriteDMA:
    0000C19A 24 26            [ 3]  150     bit *__crt0_spritePageValid
    0000C19C 10 0B            [ 4]  151     bpl __crt0_doSpriteDMA_spritePageInvalid
    0000C19E A9 00            [ 2]  152     lda #0                      ; +2
    0000C1A0 8D 03 20         [ 4]  153     sta OAMADDR                 ; +4
    0000C1A3 A9 02            [ 2]  154     lda #>_shadow_OAM           ; +2
    0000C1A5 8D 14 40         [ 4]  155     sta OAMDMA                  ; +512/513
    0000C1A8 60               [ 6]  156     rts
    0000C1A9                        157 __crt0_doSpriteDMA_spritePageInvalid:
                                    158     ; Delay 520 cycles to keep timing consistent
    0000C1A9 A2 68            [ 2]  159     ldx #104
    0000C1AB                        160 __crt0_doSpriteDMA_loop:
    0000C1AB CA               [ 2]  161     dex
    0000C1AC D0 FD            [ 4]  162     bne __crt0_doSpriteDMA_loop
    0000C1AE 60               [ 6]  163     rts
                                    164 
    0000C1AF                        165 ProcessDrawList_EndOfList:
    0000C1AF A6 4B            [ 3]  166     ldx *ProcessDrawList_tempX          ; +3
    0000C1B1 9A               [ 2]  167     txs                                 ; +2
    0000C1B2 A9 00            [ 2]  168     lda #0                              ; +2
    0000C1B4 85 38            [ 3]  169     sta *__vram_transfer_buffer_pos_w   ; +3
    0000C1B6 85 36            [ 3]  170     sta *__vram_transfer_buffer_valid   ; +3
    0000C1B8 60               [ 6]  171     rts                                 ; +6
                                    172     ; = 3 + 2 + 2 + 3 + 3 + 6 = 19
                                    173 
                                    174 ;
                                    175 ; Number of cycles spent = 19 + 21 + 48*NumTransfers + 8*NumBytesTransferred
                                    176 ;                        = 56 + 48*NumTransfers + 8*NumBytesTransferred
                                    177 ;                        = 8 * (7 + 6*NumTransfers + NumBytesTransferred)
                                    178 ;                        = 8 * (6*NumTransfers + NumBytesTransferred + 7)
                                    179 ;
    0000C1B9                        180 ProcessDrawList:
    0000C1B9 A9 C1            [ 2]  181     lda #>ProcessDrawList_UnrolledCopyLoop  ; +2
    0000C1BB 85 4A            [ 3]  182     sta *ProcessDrawList_addr+1             ; +3
    0000C1BD BA               [ 2]  183     tsx                                     ; +2
    0000C1BE 86 4B            [ 3]  184     stx *ProcessDrawList_tempX              ; +3
    0000C1C0 A2 FF            [ 2]  185     ldx #0xFF                               ; +2
    0000C1C2 9A               [ 2]  186     txs                                     ; +2
    0000C1C3 4C 81 C1         [ 3]  187     jmp ProcessDrawList_DoOneTransfer       ; +3
                                    188     ; Total = 2 + 3 + 2 + 3 + 2 + 2 + 3 = 17 fixed-cost entry
                                    189 
                         00000000   190 .ifeq GBDK_NES_EVERY_SCANLINE_IRQ 
                                    191 ;
                                    192 ; Delays until specified (non-zero) scanline is reached
                                    193 ;
                                    194 ; First scanline's delay needs adjusting in coordination with .do_lcd_ppu_reg_writes
                                    195 ;
                                    196 .define .acc "___SDCC_m6502_ret4"
                                    197 .delay_to_lcd_scanline::
                                    198     jsr .delay_12_cycles
                                    199     jmp 2$
                                    200 1$:
                                    201     jsr .delay_28_cycles
                                    202     jsr .delay_28_cycles
                                    203     jsr .delay_12_cycles ; -> 28 + 28 + 12 = 68 cycles
                                    204 2$:
                                    205 
                                    206     jsr .delay_fractional   ; -> 40.666 NTSC cycles  33.5625 PAL cycles
                                    207   
                                    208     dex
                                    209     bne 1$      ; -> 5 cycles
                                    210     rts
                                    211 
                                    212 .delay_28_cycles:
                                    213     jsr .delay_12_cycles
                                    214     nop
                                    215     nop
                                    216 .delay_12_cycles:
                                    217     rts
                                    218 
                                    219 ;
                                    220 ; Takes 40.666 NTSC cycles / 33.5626 PAL cycles
                                    221 ;
                                    222 .delay_fractional:
                                    223     lda #144 ; Initialize A with PAL fractional cycle count
                                    224     ; +7 cycles for NTSC scanlines
                                    225     bit *__SYSTEM
                                    226     bvs 3$
                                    227     lda #171 ; NTSC fractional cycle count
                                    228     nop
                                    229     nop
                                    230     nop
                                    231 3$:             ; -> 15 NTSC cycles / 8 PAL cycles
                                    232     ; Add fractional cycles and branch on carry
                                    233     clc
                                    234     adc *.acc
                                    235     sta *.acc
                                    236     bcs 4$
                                    237 4$:
                                    238     sta *.acc   ; -> 13.666 NTSC cycles / 13.5625 PAL cycles
                                    239     rts         ; -> 6 cycles for RTS, 6 cycles for JSR = 12 cycles
                                    240 
                                    241 .endif
                                    242 
    0000C1C6                        243 __crt0_NMI_earlyout:
    0000C1C6 40               [ 6]  244     rti
    0000C1C7                        245 __crt0_NMI:
    0000C1C7 24 27            [ 3]  246     bit *__crt0_disableNMI
    0000C1C9 30 FB            [ 4]  247     bmi __crt0_NMI_earlyout
    0000C1CB 48               [ 3]  248     pha
    0000C1CC 8A               [ 2]  249     txa
    0000C1CD 48               [ 3]  250     pha
    0000C1CE 98               [ 2]  251     tya
    0000C1CF 48               [ 3]  252     pha
                                    253     
                                    254     ; Skip graphics updates if blanked, to allow main code to do VRAM address / scroll updates
    0000C1D0 A5 25            [ 3]  255     lda *_shadow_PPUMASK
    0000C1D2 29 18            [ 2]  256     and #(PPUMASK_SHOW_BG | PPUMASK_SHOW_SPR)
    0000C1D4 F0 5F            [ 4]  257     beq __crt0_NMI_skip
                                    258     ; Do Sprite DMA or delay equivalent cycles
    0000C1D6 20 9A C1         [ 6]  259     jsr __crt0_doSpriteDMA
                                    260     ; Update VRAM
    0000C1D9 AD 02 20         [ 4]  261     lda PPUSTATUS
    0000C1DC A9 08            [ 2]  262     lda #PPUCTRL_SPR_CHR
    0000C1DE 8D 00 20         [ 4]  263     sta PPUCTRL
    0000C1E1 20 51 C2         [ 6]  264     jsr DoUpdateVRAM  
                                    265 
                         00000001   266 .ifne GBDK_NES_EVERY_SCANLINE_IRQ
                                    267     ; Set VADDR to 0x0000 to prevent ESI IRQs, and acknowledge any pending ESI-IRQs
    0000C1E4 2C 02 20         [ 4]  268     bit PPUSTATUS
    0000C1E7 A9 00            [ 2]  269     lda #0
    0000C1E9 8D 06 20         [ 4]  270     sta PPUADDR
    0000C1EC 8D 06 20         [ 4]  271     sta PPUADDR
    0000C1EF A5 30            [ 3]  272     lda *__vbl_isr_mapper_config
    0000C1F1 8D 00 80         [ 4]  273     sta CFG_REG
                                    274 .endif
                                    275 
                                    276     ; Select deferred-isr buffer to read from
    0000C1F4 A0 00            [ 2]  277     ldy #0
    0000C1F6 AD 74 60         [ 4]  278     lda __lcd_isr_num_calls
    0000C1F9 2C 7F 60         [ 4]  279     bit __lcd_isr_read_buf
    0000C1FC 10 05            [ 4]  280     bpl 0$
    0000C1FE A0 05            [ 2]  281     ldy #.MAX_DEFERRED_ISR_CALLS
    0000C200 AD 75 60         [ 4]  282     lda __lcd_isr_num_calls+1
    0000C203                        283 0$:
    0000C203 84 32            [ 3]  284     sty *__ESI_write_index
    0000C205 8D 76 60         [ 4]  285     sta __lcd_isr_buf_length
                                    286 
                                    287     ; Set scroll address
    0000C208 B9 42 60         [ 5]  288     lda __lcd_isr_scroll_x,y
    0000C20B 8D 05 20         [ 4]  289     sta PPUSCROLL
    0000C20E B9 4C 60         [ 5]  290     lda __lcd_isr_scroll_y,y
    0000C211 8D 05 20         [ 4]  291     sta PPUSCROLL
    0000C214 A5 00            [ 3]  292     lda *0x00
                                    293   
                                    294     ; Write PPUCTRL and force NMI enabled to avoid deadlock from buggy isr handlers
    0000C216 B9 2E 60         [ 5]  295     lda __lcd_isr_PPUCTRL,y
                                    296     ;lda *_shadow_PPUCTRL
    0000C219 09 80            [ 2]  297     ora #0x80
    0000C21B 8D 00 20         [ 4]  298     sta PPUCTRL
                                    299 
                                    300     ; Write PPUMASK, in case it was disabled
    0000C21E B9 38 60         [ 5]  301     lda __lcd_isr_PPUMASK,y
    0000C221 8D 01 20         [ 4]  302     sta PPUMASK
                                    303 
                                    304     ; Call fake LCD isr if present (0x60 = RTS means no LCD) and
    0000C224 AD 0E 60         [ 4]  305     lda .jmp_to_LCD_isr
    0000C227 C9 60            [ 2]  306     cmp #0x60
    0000C229 F0 0A            [ 4]  307     beq __crt0_NMI_skip
                         00000001   308 .ifne GBDK_NES_EVERY_SCANLINE_IRQ
                                    309     ; Setup IRQ for LCD isr
    0000C22B E6 32            [ 5]  310     inc *__ESI_write_index
    0000C22D F0 06            [ 4]  311     beq 9$
    0000C22F A6 30            [ 3]  312     ldx *__vbl_isr_mapper_config
    0000C231 20 13 C5         [ 6]  313     jsr SetupNextLCD
    0000C234 58               [ 2]  314     cli
    0000C235                        315 9$:
                         00000000   316 .else
                                    317     ; First delay until end-of-vblank, depending on transfer buffer contents...
                                    318     ; (X set to correct delay value by DoUpdateVRAM)
                                    319 1$:
                                    320     lda *0x00
                                    321     dex
                                    322     bne 1$
                                    323     ; Do additional delay of 5186 cycles if running on a PAL system, and -5*7 + 2 = -33 for alignment
                                    324     ; This is to compensate for the longer vblank period of 7459 vs NTSC's 2273
                                    325     bit *__SYSTEM
                                    326     bvc 2$
                                    327     nop
                                    328     nop
                                    329     ldy #5
                                    330     ldx #(14-7)
                                    331 3$:
                                    332     dex
                                    333     bne 3$
                                    334     dey
                                    335     bne 3$
                                    336 2$:
                                    337     ; Call the write reg subroutine
                                    338     jsr .do_lcd_ppu_reg_writes
                                    339 .endif
    0000C235                        340 __crt0_NMI_skip:
                                    341 
                                    342     ;
    0000C235 20 08 60         [ 6]  343     jsr .jmp_to_TIM_isr
                                    344     ;
                                    345 
                                    346     ; Update frame counter
    0000C238 A5 21            [ 3]  347     lda *_sys_time
    0000C23A 18               [ 2]  348     clc
    0000C23B 69 01            [ 2]  349     adc #1
    0000C23D 85 21            [ 3]  350     sta *_sys_time
    0000C23F A5 22            [ 3]  351     lda *(_sys_time+1)
    0000C241 69 00            [ 2]  352     adc #0
    0000C243 85 22            [ 3]  353     sta *(_sys_time+1)
                                    354 
                                    355     ; Restore current bank
    0000C245 A5 20            [ 3]  356     lda *__current_bank
    0000C247 A8               [ 2]  357     tay
    0000C248 99 00 C0         [ 5]  358     sta .identity,y
                                    359 
    0000C24B 68               [ 4]  360     pla
    0000C24C A8               [ 2]  361     tay
    0000C24D 68               [ 4]  362     pla
    0000C24E AA               [ 2]  363     tax
    0000C24F 68               [ 4]  364     pla
    0000C250 40               [ 6]  365     rti
                                    366 
    0000C251                        367 DoUpdateVRAM:
    00000251                        368     WRITE_PALETTE_SHADOW
    0000C251 A9 3F            [ 2]    1     lda #>0x3F00
    0000C253 8D 06 20         [ 4]    2     sta PPUADDR
    0000C256 A9 00            [ 2]    3     lda #<0x3F00
    0000C258 8D 06 20         [ 4]    4     sta PPUADDR
    0000C25B AE 13 60         [ 4]    5     ldx __crt0_paletteShadow
                         00000000     6     i = 0
                                      7 .rept 8
                                      8     stx PPUDATA
                                      9     lda (__crt0_paletteShadow+1+3*i+0)
                                     10     sta PPUDATA
                                     11     lda (__crt0_paletteShadow+1+3*i+1)
                                     12     sta PPUDATA
                                     13     lda (__crt0_paletteShadow+1+3*i+2)
                                     14     sta PPUDATA
                                     15     i = i + 1
                                     16 .endm
    0000C25E 8E 07 20         [ 4]    1     stx PPUDATA
    0000C261 AD 14 60         [ 4]    2     lda (__crt0_paletteShadow+1+3*i+0)
    0000C264 8D 07 20         [ 4]    3     sta PPUDATA
    0000C267 AD 15 60         [ 4]    4     lda (__crt0_paletteShadow+1+3*i+1)
    0000C26A 8D 07 20         [ 4]    5     sta PPUDATA
    0000C26D AD 16 60         [ 4]    6     lda (__crt0_paletteShadow+1+3*i+2)
    0000C270 8D 07 20         [ 4]    7     sta PPUDATA
                         00000001     8     i = i + 1
    0000C273 8E 07 20         [ 4]    1     stx PPUDATA
    0000C276 AD 17 60         [ 4]    2     lda (__crt0_paletteShadow+1+3*i+0)
    0000C279 8D 07 20         [ 4]    3     sta PPUDATA
    0000C27C AD 18 60         [ 4]    4     lda (__crt0_paletteShadow+1+3*i+1)
    0000C27F 8D 07 20         [ 4]    5     sta PPUDATA
    0000C282 AD 19 60         [ 4]    6     lda (__crt0_paletteShadow+1+3*i+2)
    0000C285 8D 07 20         [ 4]    7     sta PPUDATA
                         00000002     8     i = i + 1
    0000C288 8E 07 20         [ 4]    1     stx PPUDATA
    0000C28B AD 1A 60         [ 4]    2     lda (__crt0_paletteShadow+1+3*i+0)
    0000C28E 8D 07 20         [ 4]    3     sta PPUDATA
    0000C291 AD 1B 60         [ 4]    4     lda (__crt0_paletteShadow+1+3*i+1)
    0000C294 8D 07 20         [ 4]    5     sta PPUDATA
    0000C297 AD 1C 60         [ 4]    6     lda (__crt0_paletteShadow+1+3*i+2)
    0000C29A 8D 07 20         [ 4]    7     sta PPUDATA
                         00000003     8     i = i + 1
    0000C29D 8E 07 20         [ 4]    1     stx PPUDATA
    0000C2A0 AD 1D 60         [ 4]    2     lda (__crt0_paletteShadow+1+3*i+0)
    0000C2A3 8D 07 20         [ 4]    3     sta PPUDATA
    0000C2A6 AD 1E 60         [ 4]    4     lda (__crt0_paletteShadow+1+3*i+1)
    0000C2A9 8D 07 20         [ 4]    5     sta PPUDATA
    0000C2AC AD 1F 60         [ 4]    6     lda (__crt0_paletteShadow+1+3*i+2)
    0000C2AF 8D 07 20         [ 4]    7     sta PPUDATA
                         00000004     8     i = i + 1
    0000C2B2 8E 07 20         [ 4]    1     stx PPUDATA
    0000C2B5 AD 20 60         [ 4]    2     lda (__crt0_paletteShadow+1+3*i+0)
    0000C2B8 8D 07 20         [ 4]    3     sta PPUDATA
    0000C2BB AD 21 60         [ 4]    4     lda (__crt0_paletteShadow+1+3*i+1)
    0000C2BE 8D 07 20         [ 4]    5     sta PPUDATA
    0000C2C1 AD 22 60         [ 4]    6     lda (__crt0_paletteShadow+1+3*i+2)
    0000C2C4 8D 07 20         [ 4]    7     sta PPUDATA
                         00000005     8     i = i + 1
    0000C2C7 8E 07 20         [ 4]    1     stx PPUDATA
    0000C2CA AD 23 60         [ 4]    2     lda (__crt0_paletteShadow+1+3*i+0)
    0000C2CD 8D 07 20         [ 4]    3     sta PPUDATA
    0000C2D0 AD 24 60         [ 4]    4     lda (__crt0_paletteShadow+1+3*i+1)
    0000C2D3 8D 07 20         [ 4]    5     sta PPUDATA
    0000C2D6 AD 25 60         [ 4]    6     lda (__crt0_paletteShadow+1+3*i+2)
    0000C2D9 8D 07 20         [ 4]    7     sta PPUDATA
                         00000006     8     i = i + 1
    0000C2DC 8E 07 20         [ 4]    1     stx PPUDATA
    0000C2DF AD 26 60         [ 4]    2     lda (__crt0_paletteShadow+1+3*i+0)
    0000C2E2 8D 07 20         [ 4]    3     sta PPUDATA
    0000C2E5 AD 27 60         [ 4]    4     lda (__crt0_paletteShadow+1+3*i+1)
    0000C2E8 8D 07 20         [ 4]    5     sta PPUDATA
    0000C2EB AD 28 60         [ 4]    6     lda (__crt0_paletteShadow+1+3*i+2)
    0000C2EE 8D 07 20         [ 4]    7     sta PPUDATA
                         00000007     8     i = i + 1
    0000C2F1 8E 07 20         [ 4]    1     stx PPUDATA
    0000C2F4 AD 29 60         [ 4]    2     lda (__crt0_paletteShadow+1+3*i+0)
    0000C2F7 8D 07 20         [ 4]    3     sta PPUDATA
    0000C2FA AD 2A 60         [ 4]    4     lda (__crt0_paletteShadow+1+3*i+1)
    0000C2FD 8D 07 20         [ 4]    5     sta PPUDATA
    0000C300 AD 2B 60         [ 4]    6     lda (__crt0_paletteShadow+1+3*i+2)
    0000C303 8D 07 20         [ 4]    7     sta PPUDATA
                         00000008     8     i = i + 1
    0000C306 24 36            [ 3]  369     bit *__vram_transfer_buffer_valid
    0000C308 30 07            [ 4]  370     bmi DoUpdateVRAM_drawListValid
    0000C30A                        371 DoUpdateVRAM_drawListInvalid:
                                    372     ; Delay for all unused cycles and ProcessDrawList overhead to keep timing consistent
    0000C30A A5 00            [ 3]  373     lda *0x00
    0000C30C EA               [ 2]  374     nop
    0000C30D A2 AF            [ 2]  375     ldx #(VRAM_DELAY_CYCLES_X8+6)
    0000C30F D0 09            [ 4]  376     bne DoUpdateVRAM_end
    0000C311                        377 DoUpdateVRAM_drawListValid:
    0000C311 20 B9 C1         [ 6]  378     jsr ProcessDrawList
                                    379     ; Delay for remaining unused cycles to keep timing consistent
    0000C314 A6 37            [ 3]  380     ldx *__vram_transfer_buffer_num_cycles_x8
                                    381     ; Reset available cycles to initial value
    0000C316 A9 A9            [ 2]  382     lda #VRAM_DELAY_CYCLES_X8
    0000C318 85 37            [ 3]  383     sta *__vram_transfer_buffer_num_cycles_x8
    0000C31A                        384 DoUpdateVRAM_end:
    0000C31A 60               [ 6]  385     rts
                                    386 
    0000C31B                        387 __crt0_setPalette:
                                    388     ; Set background color to 30 (white)
    0000C31B A9 30            [ 2]  389     lda #0x30
    0000C31D 8D 13 60         [ 4]  390     sta __crt0_paletteShadow
                                    391     ; set all background / sprite sub-palettes to 10, 00, 1D
    0000C320 A2 18            [ 2]  392     ldx #0x18
    0000C322                        393 1$:
    0000C322 A9 1D            [ 2]  394     lda #0x1D
    0000C324 9D 13 60         [ 5]  395     sta __crt0_paletteShadow,x
    0000C327 CA               [ 2]  396     dex
    0000C328 A9 00            [ 2]  397     lda #0x00
    0000C32A 9D 13 60         [ 5]  398     sta __crt0_paletteShadow,x
    0000C32D CA               [ 2]  399     dex
    0000C32E A9 10            [ 2]  400     lda #0x10
    0000C330 9D 13 60         [ 5]  401     sta __crt0_paletteShadow,x
    0000C333 CA               [ 2]  402     dex
    0000C334 D0 EC            [ 4]  403     bne 1$
    0000C336 60               [ 6]  404     rts
                                    405 
                                    406 ;
                                    407 ; Waits for vblank flag to be set. This macro should only be
                                    408 ; used during the PPU warm-up phase at reset, as a hardware 
                                    409 ; flaw can cause the flag to be cleared in the register
                                    410 ; without returning a set flag on the CPU data bus.
                                    411 ;
                                    412 ; On Dendy-like Famiclones there is an additional problem
                                    413 ; that this pathological case can occur *every* frame if the wait
                                    414 ; loop is exactly 8 cycles long, causing a soft-lock at reset.
                                    415 ; For this reason, code before the "bpl .loop" instruction must 
                                    416 ; be aligned so the branch does not cross a 256-byte page.
                                    417 ;
                                    418 ; https://www.nesdev.org/wiki/PPU_power_up_state
                                    419 ;
                                    420 .macro CRT0_WAIT_PPU ?.loop;
                                    421 .loop:
                                    422     lda PPUSTATUS
                                    423     bpl .loop
                                    424 .endm
                                    425 
                                    426 ;
                                    427 ; Detects system. After execution, A contains the following values:
                                    428 ;
                                    429 ; 0: NTSC NES/Famicom
                                    430 ; 1: PAL NES
                                    431 ; 2: Dendy-like Famiclone
                                    432 ;
                                    433 .macro CRT0_WAIT_PPU_AND_DETECT_SYSTEM ?.loop, ?.end_of_loop, ?.end
                                    434     ldx #0
                                    435     ldy #0
                                    436 ; 256 iterations of the inner loop (X) takes 256 * (4 + 2 + 2 + 3) - 1 = 2816 cycles
                                    437 ; 1 iteration of the outer loop takes 2816 + 2 + 3 = 2821 cycles
                                    438 ; And different systems will have the following contents in Y:
                                    439 ; NTSC:   29780 / 2821 = 10
                                    440 ; PAL:    33247 / 2821 = 11
                                    441 ; Dendy:  35464 / 2821 = 12
                                    442 .loop:
                                    443     bit PPUSTATUS
                                    444     bmi .end_of_loop
                                    445     inx
                                    446     bne .loop
                                    447     iny
                                    448     bne .loop
                                    449 .end_of_loop:
                                    450     tya
                                    451     sec
                                    452     sbc #10
                                    453 .end:
                                    454 .endm
                                    455 
                                    456 .macro CRT0_CLEAR_RAM
                                    457     ; Clear WRAM
                                    458     lda #>0x6000
                                    459     sta *REGTEMP+1
                                    460     lda #<0x6000
                                    461     sta *REGTEMP
                                    462     tay
                                    463 __crt0_clearWRAM_loop:
                                    464     sta [*REGTEMP],y
                                    465     iny
                                    466     bne __crt0_clearWRAM_loop
                                    467     inc *REGTEMP+1
                                    468     bpl __crt0_clearWRAM_loop
                                    469     ; Clear console RAM
                                    470     ldx #0x00
                                    471     txa
                                    472 __crt0_clearRAM_loop:
                                    473     sta 0x0000,x
                                    474     sta 0x0100,x
                                    475     sta 0x0200,x
                                    476     sta 0x0300,x
                                    477     sta 0x0400,x
                                    478     sta 0x0500,x
                                    479     sta 0x0600,x
                                    480     sta 0x0700,x
                                    481     inx
                                    482     bne __crt0_clearRAM_loop
                                    483 .endm
                                    484 
    0000C337                        485 __crt0_clearVRAM:
    0000C337 A9 00            [ 2]  486     lda #0x00
    0000C339 A0 10            [ 2]  487     ldy #0x10
    0000C33B                        488 __crt0_clearVRAM_hi_addr_A:
    0000C33B 8D 06 20         [ 4]  489     sta PPUADDR
    0000C33E A9 00            [ 2]  490     lda #0x00
    0000C340 8D 06 20         [ 4]  491     sta PPUADDR
    0000C343 A2 00            [ 2]  492     ldx #0
    0000C345                        493 __crt0_clearVRAM_loop:
    0000C345 8D 07 20         [ 4]  494     sta PPUDATA
    0000C348 CA               [ 2]  495     dex
    0000C349 D0 FA            [ 4]  496     bne __crt0_clearVRAM_loop
    0000C34B 88               [ 2]  497     dey
    0000C34C D0 F7            [ 4]  498     bne __crt0_clearVRAM_loop
    0000C34E 60               [ 6]  499     rts
                                    500 
                         00000001   501 .ifne GBDK_NES_EVERY_SCANLINE_IRQ
    0000C34F                        502 __crt0_clearAT:
    0000C34F A0 07            [ 2]  503     ldy #7
    0000C351                        504 1$:
    0000C351 8C 00 80         [ 4]  505     sty CFG_REG
    0000C354 98               [ 2]  506     tya
    0000C355 48               [ 3]  507     pha
    0000C356 A9 0F            [ 2]  508     lda #0x0F
    0000C358 A0 01            [ 2]  509     ldy #1
    0000C35A 20 3B C3         [ 6]  510     jsr __crt0_clearVRAM_hi_addr_A 
    0000C35D 68               [ 4]  511     pla
    0000C35E A8               [ 2]  512     tay
    0000C35F 88               [ 2]  513     dey
    0000C360 10 EF            [ 4]  514     bpl 1$
    0000C362 A9 00            [ 2]  515     lda #0
    0000C364 8D 00 80         [ 4]  516     sta CFG_REG
    0000C367 60               [ 6]  517     rts
                                    518 .endif
                                    519 
    0000C368                        520 .wait_vbl_done::
    0000C368                        521 _wait_vbl_done::
    0000C368                        522 _vsync::
                                    523 
                                    524     .define .lcd_scanline_previous "REGTEMP"
                                    525     .define .lcd_num_calls "REGTEMP+1"
                                    526     .define .lcd_buf_end "REGTEMP+2"
                                    527 
    0000C368 20 C8 D4         [ 6]  528     jsr _flush_shadow_attributes
                                    529     
                                    530     ; Save shadow registers that VBL or LCD isr could change
    0000C36B A5 25            [ 3]  531     lda *_shadow_PPUMASK
    0000C36D 48               [ 3]  532     pha
    0000C36E A5 24            [ 3]  533     lda *_shadow_PPUCTRL
    0000C370 48               [ 3]  534     pha
    0000C371 A5 28            [ 3]  535     lda *_bkg_scroll_x
    0000C373 48               [ 3]  536     pha
    0000C374 A5 29            [ 3]  537     lda *_bkg_scroll_y
    0000C376 48               [ 3]  538     pha
    0000C377 A5 3A            [ 3]  539     lda *__vram_transfer_mapper_bits
    0000C379 48               [ 3]  540     pha
                                    541     
                                    542     ; Allow VBL isr to modify shadow registers if present
    0000C37A 20 0B 60         [ 6]  543     jsr .jmp_to_VBL_isr
                                    544 
                                    545     ; Set initial scanline value
    0000C37D A9 FF            [ 2]  546     lda #0xFF
    0000C37F 85 3C            [ 3]  547     sta *.lcd_scanline_previous
                                    548     ;
                                    549 
    0000C381 A0 00            [ 2]  550     ldy #0
    0000C383 A9 05            [ 2]  551     lda #.MAX_DEFERRED_ISR_CALLS
    0000C385 2C 7F 60         [ 4]  552     bit __lcd_isr_read_buf
    0000C388 30 04            [ 4]  553     bmi 8$
    0000C38A A0 05            [ 2]  554     ldy #.MAX_DEFERRED_ISR_CALLS
    0000C38C A9 0A            [ 2]  555     lda #(2*.MAX_DEFERRED_ISR_CALLS)
    0000C38E                        556 8$:
    0000C38E 84 3D            [ 3]  557     sty *.lcd_num_calls
    0000C390 85 3E            [ 3]  558     sta *.lcd_buf_end
                                    559 
                                    560     ; Special-case: LCD at scanline 0 should just directly replace first entry
    0000C392 A5 44            [ 3]  561     lda *__lcd_scanline
    0000C394 D0 07            [ 4]  562     bne 0$
    0000C396 20 0E 60         [ 6]  563     jsr .jmp_to_LCD_isr
    0000C399 A9 FF            [ 2]  564     lda #0xFF
    0000C39B 85 3C            [ 3]  565     sta *.lcd_scanline_previous
    0000C39D                        566 0$:
                                    567 
                                    568     ; Write shadow registers as first LCD entry (VBL and LCD at scanline 0 are equal)
    0000C39D A4 3D            [ 3]  569     ldy *.lcd_num_calls
    0000C39F 20 10 C4         [ 6]  570     jsr .write_shadow_registers_to_buffer
    0000C3A2 C8               [ 2]  571     iny
    0000C3A3 84 3D            [ 3]  572     sty *.lcd_num_calls
                                    573 
    0000C3A5 A5 3A            [ 3]  574     lda *__vram_transfer_mapper_bits
    0000C3A7 85 30            [ 3]  575     sta *__vbl_isr_mapper_config
                                    576 
    0000C3A9 A5 3C            [ 3]  577     lda *.lcd_scanline_previous
                                    578 
    0000C3AB 4C B1 C3         [ 3]  579     jmp 2$
    0000C3AE                        580 1$:
    0000C3AE 68               [ 4]  581     pla
    0000C3AF 85 3C            [ 3]  582     sta *.lcd_scanline_previous
    0000C3B1                        583 2$:
                                    584     ; We are done if next scanline is <= the previous one
    0000C3B1 C9 FF            [ 2]  585     cmp #0xFF
    0000C3B3 F0 04            [ 4]  586     beq 3$
    0000C3B5 C5 44            [ 3]  587     cmp *__lcd_scanline
    0000C3B7 B0 22            [ 4]  588     bcs _wait_vbl_done_waitForNextFrame
    0000C3B9                        589 3$:
                                    590     ;
    0000C3B9 A4 3D            [ 3]  591     ldy *.lcd_num_calls
    0000C3BB A5 44            [ 3]  592     lda *__lcd_scanline
                                    593     ; We are done if next LCD scanline >= SCREENHEIGHT
    0000C3BD C9 F0            [ 2]  594     cmp #.SCREENHEIGHT
    0000C3BF B0 1A            [ 4]  595     bcs _wait_vbl_done_waitForNextFrame
    0000C3C1 48               [ 3]  596     pha
    0000C3C2 18               [ 2]  597     clc ; -1 to compensate for LCD PPU write taking up a scanline on its own
    0000C3C3 E5 3C            [ 3]  598     sbc *.lcd_scanline_previous
    0000C3C5 99 56 60         [ 5]  599     sta __lcd_isr_delay_num_scanlines,y
                                    600     ; Add number of delayed scanlines to _bkg_scroll_y to simulate PPU increment
    0000C3C8 18               [ 2]  601     clc
    0000C3C9 65 29            [ 3]  602     adc *_bkg_scroll_y
    0000C3CB 85 29            [ 3]  603     sta *_bkg_scroll_y
                                    604     ; Call LCD isr
    0000C3CD 20 0E 60         [ 6]  605     jsr .jmp_to_LCD_isr
    0000C3D0 20 10 C4         [ 6]  606     jsr .write_shadow_registers_to_buffer
                                    607        
    0000C3D3 C8               [ 2]  608     iny
    0000C3D4 84 3D            [ 3]  609     sty *.lcd_num_calls
    0000C3D6 C4 3E            [ 3]  610     cpy *.lcd_buf_end
    0000C3D8 D0 D4            [ 4]  611     bne 1$
                                    612     
                                    613     ; Clear last-scanline-value from stack
    0000C3DA 68               [ 4]  614     pla
                                    615 
    0000C3DB                        616 _wait_vbl_done_waitForNextFrame:
    0000C3DB A5 3D            [ 3]  617     lda *.lcd_num_calls
    0000C3DD A0 00            [ 2]  618     ldy #0
    0000C3DF 2C 7F 60         [ 4]  619     bit __lcd_isr_read_buf
    0000C3E2 30 01            [ 4]  620     bmi 10$
    0000C3E4 C8               [ 2]  621     iny
    0000C3E5                        622 10$:
    0000C3E5 99 74 60         [ 5]  623     sta __lcd_isr_num_calls,y
                                    624     
                                    625     ; Flip read buf atomically
    0000C3E8 AD 7F 60         [ 4]  626     lda __lcd_isr_read_buf
    0000C3EB 49 80            [ 2]  627     eor #0x80
    0000C3ED 8D 7F 60         [ 4]  628     sta __lcd_isr_read_buf
                                    629     ; Enable OAM DMA in next NMI
    0000C3F0 38               [ 2]  630     sec
    0000C3F1 66 26            [ 5]  631     ror *__crt0_spritePageValid
                                    632     ; Restore shadow registers
    0000C3F3 68               [ 4]  633     pla
    0000C3F4 85 3A            [ 3]  634     sta *__vram_transfer_mapper_bits
    0000C3F6 68               [ 4]  635     pla
    0000C3F7 85 29            [ 3]  636     sta *_bkg_scroll_y
    0000C3F9 68               [ 4]  637     pla
    0000C3FA 85 28            [ 3]  638     sta *_bkg_scroll_x
    0000C3FC 68               [ 4]  639     pla
    0000C3FD 85 24            [ 3]  640     sta *_shadow_PPUCTRL
    0000C3FF 68               [ 4]  641     pla
    0000C400 85 25            [ 3]  642     sta *_shadow_PPUMASK
                                    643 
    0000C402 A5 21            [ 3]  644     lda *_sys_time
    0000C404                        645 _wait_vbl_done_waitForNextFrame_loop:
    0000C404 C5 21            [ 3]  646     cmp *_sys_time
    0000C406 F0 FC            [ 4]  647     beq _wait_vbl_done_waitForNextFrame_loop
                                    648 
                                    649     ; Disable OAM DMA in next NMI
    0000C408 18               [ 2]  650     clc
    0000C409 66 26            [ 5]  651     ror *__crt0_spritePageValid
                                    652 
                                    653     ;
    0000C40B A5 21            [ 3]  654     lda *_sys_time
    0000C40D 85 23            [ 3]  655     sta *_sys_time_old
                                    656     ;
    0000C40F 60               [ 6]  657     rts
                                    658 
    0000C410                        659 .write_shadow_registers_to_buffer:
                                    660     ; Copy shadow registers
    0000C410 A4 3D            [ 3]  661     ldy *.lcd_num_calls
    0000C412 A5 25            [ 3]  662     lda *_shadow_PPUMASK
    0000C414 99 38 60         [ 5]  663     sta __lcd_isr_PPUMASK,y
    0000C417 A5 24            [ 3]  664     lda *_shadow_PPUCTRL
    0000C419 99 2E 60         [ 5]  665     sta __lcd_isr_PPUCTRL,y
    0000C41C A5 28            [ 3]  666     lda *_bkg_scroll_x
    0000C41E 99 42 60         [ 5]  667     sta __lcd_isr_scroll_x,y
    0000C421 4A               [ 2]  668     lsr
    0000C422 4A               [ 2]  669     lsr
    0000C423 4A               [ 2]  670     lsr
    0000C424 99 6A 60         [ 5]  671     sta __lcd_isr_ppuaddr_lo,y
    0000C427 A5 29            [ 3]  672     lda *_bkg_scroll_y
    0000C429 99 4C 60         [ 5]  673     sta __lcd_isr_scroll_y,y
    0000C42C 29 F8            [ 2]  674     and #0xF8
    0000C42E 0A               [ 2]  675     asl
    0000C42F 0A               [ 2]  676     asl
    0000C430 19 6A 60         [ 5]  677     ora __lcd_isr_ppuaddr_lo,y
    0000C433 99 6A 60         [ 5]  678     sta __lcd_isr_ppuaddr_lo,y
    0000C436 A5 3A            [ 3]  679     lda *__vram_transfer_mapper_bits
    0000C438 99 60 60         [ 5]  680     sta __lcd_isr_mapper_config,y
    0000C43B 60               [ 6]  681     rts
                                    682 
    0000C43C                        683 .display_off::
    0000C43C                        684 _display_off::
    0000C43C A5 25            [ 3]  685     lda *_shadow_PPUMASK
    0000C43E 29 E7            [ 2]  686     and #~(PPUMASK_SHOW_BG | PPUMASK_SHOW_SPR)
    0000C440 85 25            [ 3]  687     sta *_shadow_PPUMASK
    0000C442 8D 01 20         [ 4]  688     sta PPUMASK
                                    689     ; Set forced blanking bit
    0000C445 38               [ 2]  690     sec
    0000C446 66 2D            [ 5]  691     ror *.crt0_forced_blanking
    0000C448 60               [ 6]  692     rts
                                    693 
    0000C449                        694 .display_on::
    0000C449                        695 _display_on::
    0000C449 A5 25            [ 3]  696     lda *_shadow_PPUMASK
    0000C44B 09 18            [ 2]  697     ora #(PPUMASK_SHOW_BG | PPUMASK_SHOW_SPR)
    0000C44D 85 25            [ 3]  698     sta *_shadow_PPUMASK
                                    699     ; Clear forced blanking bit
    0000C44F 18               [ 2]  700     clc
    0000C450 66 2D            [ 5]  701     ror *.crt0_forced_blanking
    0000C452 60               [ 6]  702     rts
                                    703 
    0000C453                        704 __crt0_RESET:
                                    705     ; Disable IRQs
    0000C453 78               [ 2]  706     sei
                                    707     ; Set stack pointer
    0000C454 A2 FF            [ 2]  708     ldx #0xff
    0000C456 9A               [ 2]  709     txs
                                    710     ; Set switchable bank to first
    0000C457                        711 __crt0_RESET_bankSwitchValue:
    0000C457 A9 00            [ 2]  712     lda #0x00
    0000C459 8D 58 C4         [ 4]  713     sta __crt0_RESET_bankSwitchValue+1
    0000C45C 8D 00 80         [ 4]  714     sta CFG_REG
                                    715     ; Disable NMIs and rendering
    0000C45F 8D 00 20         [ 4]  716     sta PPUCTRL
    0000C462 8D 01 20         [ 4]  717     sta PPUMASK
                                    718     ; Clear RAM
    00000465                        719     CRT0_CLEAR_RAM
                                      1     ; Clear WRAM
    0000C465 A9 60            [ 2]    2     lda #>0x6000
    0000C467 85 3D            [ 3]    3     sta *REGTEMP+1
    0000C469 A9 00            [ 2]    4     lda #<0x6000
    0000C46B 85 3C            [ 3]    5     sta *REGTEMP
    0000C46D A8               [ 2]    6     tay
    0000C46E                          7 __crt0_clearWRAM_loop:
    0000C46E 91 3C            [ 6]    8     sta [*REGTEMP],y
    0000C470 C8               [ 2]    9     iny
    0000C471 D0 FB            [ 4]   10     bne __crt0_clearWRAM_loop
    0000C473 E6 3D            [ 5]   11     inc *REGTEMP+1
    0000C475 10 F7            [ 4]   12     bpl __crt0_clearWRAM_loop
                                     13     ; Clear console RAM
    0000C477 A2 00            [ 2]   14     ldx #0x00
    0000C479 8A               [ 2]   15     txa
    0000C47A                         16 __crt0_clearRAM_loop:
    0000C47A 95 00            [ 4]   17     sta 0x0000,x
    0000C47C 9D 00 01         [ 5]   18     sta 0x0100,x
    0000C47F 9D 00 02         [ 5]   19     sta 0x0200,x
    0000C482 9D 00 03         [ 5]   20     sta 0x0300,x
    0000C485 9D 00 04         [ 5]   21     sta 0x0400,x
    0000C488 9D 00 05         [ 5]   22     sta 0x0500,x
    0000C48B 9D 00 06         [ 5]   23     sta 0x0600,x
    0000C48E 9D 00 07         [ 5]   24     sta 0x0700,x
    0000C491 E8               [ 2]   25     inx
    0000C492 D0 E6            [ 4]   26     bne __crt0_clearRAM_loop
                                    720     ; Wait for PPU warm-up / detect system
    0000C494 2C 02 20         [ 4]  721     bit PPUSTATUS
    0000C497                        722     CRT0_WAIT_PPU
    00000497                          1 10000$:
    0000C497 AD 02 20         [ 4]    2     lda PPUSTATUS
    0000C49A 10 FB            [ 4]    3     bpl 10000$
    0000049C                        723     CRT0_WAIT_PPU_AND_DETECT_SYSTEM
    0000C49C A2 00            [ 2]    1     ldx #0
    0000C49E A0 00            [ 2]    2     ldy #0
                                      3 ; 256 iterations of the inner loop (X) takes 256 * (4 + 2 + 2 + 3) - 1 = 2816 cycles
                                      4 ; 1 iteration of the outer loop takes 2816 + 2 + 3 = 2821 cycles
                                      5 ; And different systems will have the following contents in Y:
                                      6 ; NTSC:   29780 / 2821 = 10
                                      7 ; PAL:    33247 / 2821 = 11
                                      8 ; Dendy:  35464 / 2821 = 12
    0000C4A0                          9 10001$:
    0000C4A0 2C 02 20         [ 4]   10     bit PPUSTATUS
    0000C4A3 30 06            [ 4]   11     bmi 10002$
    0000C4A5 E8               [ 2]   12     inx
    0000C4A6 D0 F8            [ 4]   13     bne 10001$
    0000C4A8 C8               [ 2]   14     iny
    0000C4A9 D0 F5            [ 4]   15     bne 10001$
    0000C4AB                         16 10002$:
    0000C4AB 98               [ 2]   17     tya
    0000C4AC 38               [ 2]   18     sec
    0000C4AD E9 0A            [ 2]   19     sbc #10
    0000C4AF                         20 10003$:
                                    724     ; Store system in upper two bits of __SYSTEM, to allow bit instruction to quickly test for PAL
    0000C4AF 18               [ 2]  725     clc
    0000C4B0 6A               [ 2]  726     ror
    0000C4B1 6A               [ 2]  727     ror
    0000C4B2 6A               [ 2]  728     ror
    0000C4B3 85 2E            [ 3]  729     sta *__SYSTEM
                                    730     ; Clear VRAM
    0000C4B5 20 37 C3         [ 6]  731     jsr __crt0_clearVRAM
    0000C4B8 20 4F C3         [ 6]  732     jsr __crt0_clearAT
                                    733     ; Hide sprites in shadow OAM, and perform OAM DMA
    0000C4BB A2 00            [ 2]  734     ldx #0
    0000C4BD 8A               [ 2]  735     txa
    0000C4BE 20 4E D6         [ 6]  736     jsr _hide_sprites_range
    0000C4C1 8E 03 20         [ 4]  737     stx OAMADDR
    0000C4C4 A9 02            [ 2]  738     lda #>_shadow_OAM
    0000C4C6 8D 14 40         [ 4]  739     sta OAMDMA
                                    740 
                                    741     ; Perform initialization of DATA area
    0000C4C9 A9 76            [ 2]  742     lda #<s__XINIT
    0000C4CB 8D 00 00         [ 4]  743     sta ___memcpy_PARM_2
    0000C4CE A9 DB            [ 2]  744     lda #>s__XINIT
    0000C4D0 8D 01 00         [ 4]  745     sta ___memcpy_PARM_2+1
    0000C4D3 A9 13            [ 2]  746     lda #<l__XINIT
    0000C4D5 8D 02 00         [ 4]  747     sta ___memcpy_PARM_3
    0000C4D8 A9 00            [ 2]  748     lda #>l__XINIT
    0000C4DA 8D 03 00         [ 4]  749     sta ___memcpy_PARM_3+1
    0000C4DD A9 00            [ 2]  750     lda #<s__DATA
    0000C4DF A2 60            [ 2]  751     ldx #>s__DATA
    0000C4E1 20 95 C8         [ 6]  752     jsr ___memcpy
    0000C4E4 A9 20            [ 2]  753     lda #>0x2000
    0000C4E6 8D 3B 00         [ 4]  754     sta __vram_transfer_ppu_hi_mask
                                    755 
                                    756     ; Set bank to first
    0000C4E9 A9 00            [ 2]  757     lda #0x00
    0000C4EB 85 20            [ 3]  758     sta *__current_bank
                                    759     ; Set palette shadow
    0000C4ED 20 1B C3         [ 6]  760     jsr __crt0_setPalette
    0000C4F0 A9 A9            [ 2]  761     lda #VRAM_DELAY_CYCLES_X8
    0000C4F2 85 37            [ 3]  762     sta *__vram_transfer_buffer_num_cycles_x8
    0000C4F4 A9 00            [ 2]  763     lda #0
    0000C4F6 85 38            [ 3]  764     sta *__vram_transfer_buffer_pos_w
                                    765     ; 
    0000C4F8 A9 1E            [ 2]  766     lda #(PPUMASK_SHOW_BG | PPUMASK_SHOW_SPR | PPUMASK_SHOW_BG_LC | PPUMASK_SHOW_SPR_LC)
    0000C4FA 85 25            [ 3]  767     sta *_shadow_PPUMASK
    0000C4FC A9 80            [ 2]  768     lda #0x80
    0000C4FE 85 26            [ 3]  769     sta *__crt0_spritePageValid
                                    770     ; enable NMI
    0000C500 A9 88            [ 2]  771     lda #(PPUCTRL_NMI | PPUCTRL_SPR_CHR)
    0000C502 85 24            [ 3]  772     sta *_shadow_PPUCTRL
    0000C504 8D 00 20         [ 4]  773     sta PPUCTRL
                                    774     ; Turn off frame IRQ to avoid clashes with every-scanline-IRQ
    0000C507 A9 C0            [ 2]  775     lda #0xC0
    0000C509 8D 17 40         [ 4]  776     sta 0x4017
                                    777     ; Enable IRQs
    0000C50C 58               [ 2]  778     cli
                                    779     ; Call main
    0000C50D 20 DE C5         [ 6]  780     jsr _main
                                    781     ; main finished - loop forever
    0000C510                        782 __crt0_waitForever:
    0000C510 4C 10 C5         [ 3]  783     jmp __crt0_waitForever
                                    784 
                         00000000   785 .ifeq GBDK_NES_EVERY_SCANLINE_IRQ
                                    786 ;
                                    787 ; Use timed code for LCD ISR PPU register writes
                                    788 ;
                                    789 .do_lcd_ppu_reg_writes:
                                    790     .define .reg_write_index    "__crt0_NMITEMP+1"
                                    791     .define .lda_PPUADDR        "__crt0_NMITEMP+2"
                                    792     .define .ldx_PPUMASK        "__crt0_NMITEMP+3"
                                    793     
                                    794     nop
                                    795     nop
                                    796     nop
                                    797     nop
                                    798     nop
                                    799     nop
                                    800     nop
                                    801     nop
                                    802 
                                    803     ; Skip if empty buffer (no calls were made within frame)
                                    804     lda __lcd_isr_buf_length
                                    805     beq 2$
                                    806 
                                    807     ldy #0
                                    808     sty *.acc
                                    809 1$:
                                    810     sty *.reg_write_index
                                    811     ldx __lcd_isr_delay_num_scanlines,y
                                    812     beq 3$
                                    813     jsr .delay_to_lcd_scanline
                                    814 3$:
                                    815     ldy *.reg_write_index
                                    816     
                                    817 
                                    818     ; Pre-write PPUADDR (1st write) and y-scroll
                                    819     sty PPUADDR
                                    820     lda __lcd_isr_scroll_y,y
                                    821     sta PPUSCROLL
                                    822     ; A <- PPUADDR (2nd write)
                                    823     lda __lcd_isr_ppuaddr_lo,y    ; lda __lcd_isr_scroll_x,y
                                    824     sta *.lda_PPUADDR
                                    825     ; ldx <- mapper_config
                                    826     ldx __lcd_isr_mapper_config,y
                                    827     stx *.ldx_mapper_config
                                    828     ; ldx <- PPUMASK
                                    829     ldx __lcd_isr_PPUMASK,y
                                    830     stx *.ldx_PPUMASK
                                    831     ; X <- SCROLLX
                                    832     ldx __lcd_isr_scroll_x,y
                                    833     ; Y <- PPUCTRL
                                    834     lda __lcd_isr_PPUCTRL,y
                                    835     tay
                                    836     lda *.lda_PPUADDR
                                    837     ;
                                    838     ; Write 4 PPU registers in following order.
                                    839     ;
                                    840     ; 1. PPUSCROLL          (needs to be written to set fine-x)
                                    841     ; 2. PPUADDR 2nd write  (highest priority as needs to happen before the two-tile pre-fetch) 
                                    842     ; 3. PPUCTRL            (PPU pattern table switch can affect two-tile pre-fetch)
                                    843     ; 4. PPUMASK            (emphasis and render on/off are maybe less distracting?)
                                    844     ;
                                    845     ; TODO: Self-modifying code could build a non-redundant write sequence in RAM.
                                    846     ;
                                    847     stx PPUSCROLL
                                    848     sta PPUADDR
                                    849     ;lax *.ldx_mapper_config
                                    850     ;.db 0xA7, <.ldx_mapper_config
                                    851     ;txa
                                    852     sta ;.identity,x
                                    853     lda *.ldx_mapper_config
                                    854     sta CFG_REG
                                    855     ldx *.ldx_PPUMASK
                                    856     stx PPUMASK
                                    857     sty PPUCTRL
                                    858 
                                    859     sta __CFG_REG_cache
                                    860 
                                    861     ; Delay for 40.666 NTSC cycles / 33.5625 PAL cycles
                                    862     jsr .delay_fractional
                                    863     ldy *.reg_write_index
                                    864     
                                    865     ; Finally, write Y-scroll part of T with original non-LCD shadow values, but 
                                    866     ; *without* triggering an update of V, to mitigate glitches on lag frames.
                                    867     ; In normal circumstances, NMI will re-write T with the new proper Y-scroll 
                                    868     ; value for start of screen. Or the next iteration of this loop may overwrite
                                    869     ; it as well.
                                    870     ; But if our calls to VBL/LCD handlers disable NMI just at the wrong moment in
                                    871     ; the vsync routine, and cause the scroll update in NMI to be skipped, 
                                    872     ; this mitigation will leave T with a "reasonable" value of the old shadow 
                                    873     ; bkg scroll register for Y at scanline 0.
                                    874     ;NOP
                                    875     ;NOP
                                    876     lda *.ldx_mapper_config
                                    877     
                                    878     sty PPUADDR
                                    879     lda *_bkg_scroll_y
                                    880     sta PPUSCROLL
                                    881 
                                    882     iny
                                    883     cpy __lcd_isr_buf_length
                                    884     bne 1$
                                    885 2$:
                                    886     rts
                                    887 
                                    888 __crt0_IRQ:
                                    889     jmp __crt0_IRQ
                         00000001   890 .else
                                    891 ;
                                    892 ; Use every-scanline-IRQ for LCD ISR PPU register writes
                                    893 ;
    0000C513                        894 SetupNextLCD:
                                    895     .define .lda_PPUADDR        "__crt0_NMITEMP+2"
                                    896     .define .ldx_PPUMASK        "__crt0_NMITEMP+3"
                                    897     ; Skip if empty buffer (no calls were made within frame)
    0000C513 AD 76 60         [ 4]  898     lda __lcd_isr_buf_length
    0000C516 F0 18            [ 4]  899     beq 3$
                                    900     
    0000C518 A4 32            [ 3]  901     ldy *__ESI_write_index
    0000C51A CC 76 60         [ 4]  902     cpy __lcd_isr_buf_length
    0000C51D F0 11            [ 4]  903     beq 3$
                                    904     
    0000C51F B9 56 60         [ 5]  905     lda __lcd_isr_delay_num_scanlines,y
    0000C522 F0 0C            [ 4]  906     beq 3$
    0000C524 85 31            [ 3]  907     sta *__ESI_scanline_counter
    0000C526                        908 2$:
                                    909     ; At least one more split - enable every-scanline-IRQ
    0000C526 8A               [ 2]  910     txa
    0000C527 09 80            [ 2]  911     ora #CFG_IRQ_ENABLE
    0000C529 8D 33 00         [ 4]  912     sta __CFG_REG_cache
    0000C52C 8D 00 80         [ 4]  913     sta CFG_REG
    0000C52F 60               [ 6]  914     rts
                                    915 
    0000C530                        916 3$:
                                    917     ; No more splits - turn off every-scanline-IRQ
    0000C530 8A               [ 2]  918     txa
    0000C531 29 7F            [ 2]  919     and #~CFG_IRQ_ENABLE
    0000C533 8D 33 00         [ 4]  920     sta __CFG_REG_cache
    0000C536 8D 00 80         [ 4]  921     sta CFG_REG
    0000C539 60               [ 6]  922     rts
                                    923 
    0000C53A                        924 __crt0_IRQ:
    0000C53A C6 31            [ 5]  925     dec *__ESI_scanline_counter
    0000C53C F0 0A            [ 4]  926     beq __crt0_IRQ_reached_scanline
    0000C53E 85 4B            [ 3]  927     sta *__crt0_NMITEMP+2
    0000C540 A5 33            [ 3]  928     lda *__CFG_REG_cache
    0000C542 8D 00 80         [ 4]  929     sta CFG_REG
    0000C545 A5 4B            [ 3]  930     lda *__crt0_NMITEMP+2
    0000C547 40               [ 6]  931     rti
    0000C548                        932 __crt0_IRQ_reached_scanline:
    0000C548 48               [ 3]  933     pha
    0000C549 8A               [ 2]  934     txa
    0000C54A 48               [ 3]  935     pha
    0000C54B 98               [ 2]  936     tya
    0000C54C 48               [ 3]  937     pha
                                    938 
    0000C54D A0 05            [ 2]  939     ldy #5
    0000C54F                        940 1$:
    0000C54F 88               [ 2]  941     dey
    0000C550 D0 FD            [ 4]  942     bne 1$
    0000C552 EA               [ 2]  943     nop
                                    944 
    0000C553 A4 32            [ 3]  945     ldy *__ESI_write_index
                                    946 
                                    947     ; Pre-write PPUADDR (1st write) and y-scroll
    0000C555 8C 06 20         [ 4]  948     sty PPUADDR
    0000C558 B9 4C 60         [ 5]  949     lda __lcd_isr_scroll_y,y
    0000C55B 8D 05 20         [ 4]  950     sta PPUSCROLL
                                    951     ; A <- PPUADDR (2nd write)
    0000C55E B9 6A 60         [ 5]  952     lda __lcd_isr_ppuaddr_lo,y
    0000C561 85 4B            [ 3]  953     sta *.lda_PPUADDR
                                    954     ; ldx <- mapper_config
    0000C563 BE 60 60         [ 5]  955     ldx __lcd_isr_mapper_config,y
    0000C566 86 2F            [ 3]  956     stx *.ldx_mapper_config
                                    957     ; ldx <- PPUMASK
    0000C568 BE 38 60         [ 5]  958     ldx __lcd_isr_PPUMASK,y
    0000C56B 86 4C            [ 3]  959     stx *.ldx_PPUMASK
                                    960     ; X <- SCROLLX
    0000C56D BE 42 60         [ 5]  961     ldx __lcd_isr_scroll_x,y
                                    962     ; Y <- PPUCTRL
    0000C570 B9 2E 60         [ 5]  963     lda __lcd_isr_PPUCTRL,y
    0000C573 A8               [ 2]  964     tay
    0000C574 A5 4B            [ 3]  965     lda *.lda_PPUADDR
                                    966     ;
                                    967     ; Write 4 PPU registers in following order.
                                    968     ;
                                    969     ; 1. PPUSCROLL          (needs to be written to set fine-x)
                                    970     ; 2. PPUADDR 2nd write  (highest priority as needs to happen before the two-tile pre-fetch) 
                                    971     ; 3. PPUCTRL            (PPU pattern table switch can affect two-tile pre-fetch)
                                    972     ; 4. PPUMASK            (emphasis and render on/off are maybe less distracting?)
                                    973     ;
                                    974     ; TODO: Self-modifying code could build a non-redundant write sequence in RAM.
                                    975     ;
    0000C576 8E 05 20         [ 4]  976     stx PPUSCROLL
    0000C579 8D 06 20         [ 4]  977     sta PPUADDR
    0000C57C 8C 00 20         [ 4]  978     sty PPUCTRL
    0000C57F A5 2F            [ 3]  979     lda *.ldx_mapper_config
    0000C581 8D 00 80         [ 4]  980     sta CFG_REG
    0000C584 A6 4C            [ 3]  981     ldx *.ldx_PPUMASK
    0000C586 8E 01 20         [ 4]  982     stx PPUMASK
    0000C589 AA               [ 2]  983     tax
                                    984 
    0000C58A E6 32            [ 5]  985     inc *__ESI_write_index
    0000C58C 20 13 C5         [ 6]  986     jsr SetupNextLCD
                                    987 
    0000C58F 68               [ 4]  988     pla
    0000C590 A8               [ 2]  989     tay
    0000C591 68               [ 4]  990     pla
    0000C592 AA               [ 2]  991     tax
    0000C593 68               [ 4]  992     pla
    0000C594 40               [ 6]  993     rti
                                    994     
                                    995 .endif
                                    996 
                                    997 ; Interrupt / RESET vector table
                                    998 .area VECTORS (ABS)
    0000FFFA                        999 .org 0xfffa
    0000FFFA C7 C1                 1000 .dw	__crt0_NMI
    0000FFFC 53 C4                 1001 .dw	__crt0_RESET
    0000FFFE 3A C5                 1002 .dw	__crt0_IRQ
