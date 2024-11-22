                                      1 ;--------------------------------------------------------
                                      2 ; File Created by SDCC : free open source ISO C Compiler 
                                      3 ; Version 4.4.1 #14650 (MINGW64)
                                      4 ;--------------------------------------------------------
                                      5 	.module text_scroller
                                      6 	.optsdcc -mmos6502
                                      7 	
                                      8 ;; Ordering of segments for the linker.
                                      9 	.area _ZP      (PAG)
                                     10 	.area _OSEG    (PAG, OVR)
                                     11 	.area _HOME
                                     12 	.area _GSINIT
                                     13 	.area _GSFINAL
                                     14 	.area _CODE
                                     15 	.area _XINIT
                                     16 	.area _DATA
                                     17 	.area _DATA
                                     18 	.area _BSS
                                     19 ;--------------------------------------------------------
                                     20 ; Public variables in this module
                                     21 ;--------------------------------------------------------
                                     22 	.globl _scroller_text
                                     23 	.globl _scanline_offsets_tbl
                                     24 	.globl _main
                                     25 	.globl _scanline_isr
                                     26 	.globl _printf
                                     27 	.globl _font_set
                                     28 	.globl _font_load
                                     29 	.globl _font_init
                                     30 	.globl _fill_bkg_rect
                                     31 	.globl _get_bkg_xy_addr
                                     32 	.globl _set_vram_byte
                                     33 	.globl _display_off
                                     34 	.globl _display_on
                                     35 	.globl _vsync
                                     36 	.globl _add_LCD
                                     37 	.globl _scroller_next_char
                                     38 	.globl _scroller_x
                                     39 	.globl _scanline_offsets
                                     40 	.globl _limit
                                     41 	.globl _base
                                     42 	.globl _scroller_vram_addr
                                     43 	.globl _OAMDMA
                                     44 	.globl _PPUDATA
                                     45 	.globl _PPUADDR
                                     46 	.globl _PPUSCROLL
                                     47 	.globl _OAMDATA
                                     48 	.globl _OAMADDR
                                     49 	.globl _PPUSTATUS
                                     50 	.globl _PPUMASK
                                     51 	.globl _PPUCTRL
                                     52 ;--------------------------------------------------------
                                     53 ; ZP ram data
                                     54 ;--------------------------------------------------------
                                     55 	.area _ZP      (PAG)
                         00000000    56 Ltext_scroller.main$sloc0$0_1$0==.
    00000034                         57 _main_sloc0_1_0:
    00000034                         58 	.ds 2
                                     59 ;--------------------------------------------------------
                                     60 ; overlayable items in ram
                                     61 ;--------------------------------------------------------
                                     62 ;--------------------------------------------------------
                                     63 ; uninitialized external ram data
                                     64 ;--------------------------------------------------------
                                     65 	.area _BSS
                         00002000    66 G$PPUCTRL$0_0$0 == 0x2000
                         00002000    67 _PPUCTRL	=	0x2000
                         00002001    68 G$PPUMASK$0_0$0 == 0x2001
                         00002001    69 _PPUMASK	=	0x2001
                         00002002    70 G$PPUSTATUS$0_0$0 == 0x2002
                         00002002    71 _PPUSTATUS	=	0x2002
                         00002003    72 G$OAMADDR$0_0$0 == 0x2003
                         00002003    73 _OAMADDR	=	0x2003
                         00002004    74 G$OAMDATA$0_0$0 == 0x2004
                         00002004    75 _OAMDATA	=	0x2004
                         00002005    76 G$PPUSCROLL$0_0$0 == 0x2005
                         00002005    77 _PPUSCROLL	=	0x2005
                         00002006    78 G$PPUADDR$0_0$0 == 0x2006
                         00002006    79 _PPUADDR	=	0x2006
                         00002007    80 G$PPUDATA$0_0$0 == 0x2007
                         00002007    81 _PPUDATA	=	0x2007
                         00004014    82 G$OAMDMA$0_0$0 == 0x4014
                         00004014    83 _OAMDMA	=	0x4014
                         00000000    84 G$scroller_vram_addr$0_0$0==.
    00006080                         85 _scroller_vram_addr::
    00006080                         86 	.ds 2
                         00000002    87 G$base$0_0$0==.
    00006082                         88 _base::
    00006082                         89 	.ds 2
                         00000004    90 G$limit$0_0$0==.
    00006084                         91 _limit::
    00006084                         92 	.ds 2
                                     93 ;--------------------------------------------------------
                                     94 ; absolute external ram data
                                     95 ;--------------------------------------------------------
                                     96 	.area _DABS    (ABS)
                                     97 ;--------------------------------------------------------
                                     98 ; initialized external ram data
                                     99 ;--------------------------------------------------------
                                    100 	.area _DATA
                         00000000   101 G$scanline_offsets$0_0$0==.
    00006000                        102 _scanline_offsets::
    00006000                        103 	.ds 2
                         00000002   104 G$scroller_x$0_0$0==.
    00006002                        105 _scroller_x::
    00006002                        106 	.ds 1
                         00000003   107 G$scroller_next_char$0_0$0==.
    00006003                        108 _scroller_next_char::
    00006003                        109 	.ds 2
                                    110 ;--------------------------------------------------------
                                    111 ; global & static initialisations
                                    112 ;--------------------------------------------------------
                                    113 	.area _HOME
                                    114 	.area _GSINIT
                                    115 	.area _GSFINAL
                                    116 	.area _GSINIT
                                    117 ;--------------------------------------------------------
                                    118 ; Home
                                    119 ;--------------------------------------------------------
                                    120 	.area _HOME
                                    121 	.area _HOME
                                    122 ;--------------------------------------------------------
                                    123 ; code
                                    124 ;--------------------------------------------------------
                                    125 	.area _CODE
                                    126 ;------------------------------------------------------------
                                    127 ;Allocation info for local variables in function 'scanline_isr'
                                    128 ;------------------------------------------------------------
                                    129 ;__300000006               Allocated to registers 
                                    130 ;__300000007               Allocated to registers 
                                    131 ;x                         Allocated to registers 
                                    132 ;y                         Allocated to registers 
                                    133 ;__300000009               Allocated to registers 
                                    134 ;__300000010               Allocated to registers 
                                    135 ;x                         Allocated to registers 
                                    136 ;y                         Allocated to registers 
                                    137 ;__300000012               Allocated to registers 
                                    138 ;__300000013               Allocated to registers 
                                    139 ;x                         Allocated to registers 
                                    140 ;y                         Allocated to registers 
                                    141 ;------------------------------------------------------------
                         00000000   142 	G$scanline_isr$0$0 ==.
                         00000000   143 	C$text_scroller.c$16$0_0$101 ==.
                                    144 ;	src/text_scroller.c: 16: void scanline_isr(void) {
                                    145 ;	-----------------------------------------
                                    146 ;	 function scanline_isr
                                    147 ;	-----------------------------------------
                                    148 ;	Register assignment is optimal.
                                    149 ;	Stack space usage: 0 bytes.
    0000C595                        150 _scanline_isr:
                         00000000   151 	C$text_scroller.c$18$1_0$101 ==.
                                    152 ;	src/text_scroller.c: 18: switch (_lcd_scanline) {
    0000C595 AD 44 00         [ 4]  153 	lda	__lcd_scanline
    0000C598 F0 11            [ 4]  154 	beq	00101$
    0000C59A AD 44 00         [ 4]  155 	lda	__lcd_scanline
    0000C59D C9 78            [ 2]  156 	cmp	#0x78
    0000C59F F0 1A            [ 4]  157 	beq	00102$
    0000C5A1 AD 44 00         [ 4]  158 	lda	__lcd_scanline
    0000C5A4 C9 80            [ 2]  159 	cmp	#0x80
    0000C5A6 F0 26            [ 4]  160 	beq	00103$
    0000C5A8 4C DD C5         [ 3]  161 	jmp	00108$
                         00000016   162 	C$text_scroller.c$19$2_0$102 ==.
                                    163 ;	src/text_scroller.c: 19: case 0: 
    0000C5AB                        164 00101$:
                                    165 ;	c:\geeky\gbdk-2020\build\gbdk\include\nes\nes.h: 939: bkg_scroll_x = x, bkg_scroll_y = y;
    0000C5AB A2 00            [ 2]  166 	ldx	#0x00
    0000C5AD 8E 28 00         [ 4]  167 	stx	_bkg_scroll_x
    0000C5B0 8E 29 00         [ 4]  168 	stx	_bkg_scroll_y
                         0000001E   169 	C$text_scroller.c$21$2_0$102 ==.
                                    170 ;	src/text_scroller.c: 21: _lcd_scanline = SCROLL_POS_PIX_START + 1;
    0000C5B3 A2 78            [ 2]  171 	ldx	#0x78
    0000C5B5 8E 44 00         [ 4]  172 	stx	__lcd_scanline
                         00000023   173 	C$text_scroller.c$22$2_0$102 ==.
                                    174 ;	src/text_scroller.c: 22: break;
    0000C5B8 4C DD C5         [ 3]  175 	jmp	00108$
                         00000026   176 	C$text_scroller.c$23$2_0$102 ==.
                                    177 ;	src/text_scroller.c: 23: case SCROLL_POS_PIX_START + 1:
    0000C5BB                        178 00102$:
                                    179 ;	src/text_scroller.c: 24: move_bkg(scroller_x, SCROLL_POS_PIX_START + 1);
    0000C5BB AD 02 60         [ 4]  180 	lda	_scroller_x
    0000C5BE 8D 28 00         [ 4]  181 	sta	_bkg_scroll_x
                                    182 ;	c:\geeky\gbdk-2020\build\gbdk\include\nes\nes.h: 939: bkg_scroll_x = x, bkg_scroll_y = y;
    0000C5C1 A2 78            [ 2]  183 	ldx	#0x78
    0000C5C3 8E 29 00         [ 4]  184 	stx	_bkg_scroll_y
                         00000031   185 	C$text_scroller.c$25$2_0$102 ==.
                                    186 ;	src/text_scroller.c: 25: _lcd_scanline = SCROLL_POS_PIX_END + 1;
    0000C5C6 A2 80            [ 2]  187 	ldx	#0x80
    0000C5C8 8E 44 00         [ 4]  188 	stx	__lcd_scanline
                         00000036   189 	C$text_scroller.c$26$2_0$102 ==.
                                    190 ;	src/text_scroller.c: 26: break;
    0000C5CB 4C DD C5         [ 3]  191 	jmp	00108$
                         00000039   192 	C$text_scroller.c$27$2_0$102 ==.
                                    193 ;	src/text_scroller.c: 27: case SCROLL_POS_PIX_END + 1:
    0000C5CE                        194 00103$:
                                    195 ;	c:\geeky\gbdk-2020\build\gbdk\include\nes\nes.h: 939: bkg_scroll_x = x, bkg_scroll_y = y;
    0000C5CE A2 00            [ 2]  196 	ldx	#0x00
    0000C5D0 8E 28 00         [ 4]  197 	stx	_bkg_scroll_x
    0000C5D3 A2 80            [ 2]  198 	ldx	#0x80
    0000C5D5 8E 29 00         [ 4]  199 	stx	_bkg_scroll_y
                         00000043   200 	C$text_scroller.c$29$2_0$102 ==.
                                    201 ;	src/text_scroller.c: 29: _lcd_scanline = 0;
    0000C5D8 A2 00            [ 2]  202 	ldx	#0x00
    0000C5DA 8E 44 00         [ 4]  203 	stx	__lcd_scanline
                         00000048   204 	C$text_scroller.c$31$1_0$101 ==.
                                    205 ;	src/text_scroller.c: 31: }
    0000C5DD                        206 00108$:
                         00000048   207 	C$text_scroller.c$55$1_0$101 ==.
                                    208 ;	src/text_scroller.c: 55: }
                         00000048   209 	C$text_scroller.c$55$1_0$101 ==.
                         00000048   210 	XG$scanline_isr$0$0 ==.
    0000C5DD 60               [ 6]  211 	rts
                                    212 ;------------------------------------------------------------
                                    213 ;Allocation info for local variables in function 'main'
                                    214 ;------------------------------------------------------------
                                    215 ;sloc0                     Allocated with name '_main_sloc0_1_0'
                                    216 ;__300000015               Allocated to registers 
                                    217 ;__300000016               Allocated to registers 
                                    218 ;x                         Allocated to registers 
                                    219 ;y                         Allocated to registers 
                                    220 ;------------------------------------------------------------
                         00000049   221 	G$main$0$0 ==.
                         00000049   222 	C$text_scroller.c$66$1_0$113 ==.
                                    223 ;	src/text_scroller.c: 66: void main(void) {
                                    224 ;	-----------------------------------------
                                    225 ;	 function main
                                    226 ;	-----------------------------------------
                                    227 ;	Register assignment is optimal.
                                    228 ;	Stack space usage: 0 bytes.
    0000C5DE                        229 _main:
                         00000049   230 	C$text_scroller.c$67$1_0$113 ==.
                                    231 ;	src/text_scroller.c: 67: DISPLAY_OFF;
    0000C5DE 20 3C C4         [ 6]  232 	jsr	_display_off
                         0000004C   233 	C$text_scroller.c$69$1_0$113 ==.
                                    234 ;	src/text_scroller.c: 69: font_init();
    0000C5E1 20 F0 D2         [ 6]  235 	jsr	_font_init
                         0000004F   236 	C$text_scroller.c$70$1_0$113 ==.
                                    237 ;	src/text_scroller.c: 70: font_set(font_load(font_ibm));
    0000C5E4 A2 D7            [ 2]  238 	ldx	#>_font_ibm
    0000C5E6 A9 C4            [ 2]  239 	lda	#_font_ibm
    0000C5E8 20 E9 D2         [ 6]  240 	jsr	_font_load
    0000C5EB 20 EC D2         [ 6]  241 	jsr	_font_set
                         00000059   242 	C$text_scroller.c$72$1_0$113 ==.
                                    243 ;	src/text_scroller.c: 72: fill_bkg_rect(0, 0, DEVICE_SCREEN_WIDTH, DEVICE_SCREEN_HEIGHT, '*' - ' ');
    0000C5EE A2 20            [ 2]  244 	ldx	#0x20
    0000C5F0 8E 10 00         [ 4]  245 	stx	_fill_bkg_rect_PARM_3
    0000C5F3 A2 1E            [ 2]  246 	ldx	#0x1e
    0000C5F5 8E 11 00         [ 4]  247 	stx	_fill_bkg_rect_PARM_4
    0000C5F8 A2 0A            [ 2]  248 	ldx	#0x0a
    0000C5FA 8E 12 00         [ 4]  249 	stx	_fill_bkg_rect_PARM_5
    0000C5FD A9 00            [ 2]  250 	lda	#0x00
    0000C5FF AA               [ 2]  251 	tax
    0000C600 20 B1 D3         [ 6]  252 	jsr	_fill_bkg_rect
                         0000006E   253 	C$text_scroller.c$73$1_0$113 ==.
                                    254 ;	src/text_scroller.c: 73: DISPLAY_ON;
    0000C603 20 49 C4         [ 6]  255 	jsr	_display_on
                         00000071   256 	C$text_scroller.c$75$1_0$113 ==.
                                    257 ;	src/text_scroller.c: 75: printf(" Scrolling %d chars", sizeof(scroller_text) - 1);
    0000C606 A9 01            [ 2]  258 	lda	#0x01
    0000C608 48               [ 3]  259 	pha
    0000C609 A9 68            [ 2]  260 	lda	#0x68
    0000C60B 48               [ 3]  261 	pha
    0000C60C A9 C8            [ 2]  262 	lda	#>___str_0
    0000C60E 48               [ 3]  263 	pha
    0000C60F A9 81            [ 2]  264 	lda	#___str_0
    0000C611 48               [ 3]  265 	pha
    0000C612 20 F3 C8         [ 6]  266 	jsr	_printf
    0000C615 68               [ 4]  267 	pla
    0000C616 68               [ 4]  268 	pla
    0000C617 68               [ 4]  269 	pla
    0000C618 68               [ 4]  270 	pla
                         00000084   271 	C$text_scroller.c$82$1_0$113 ==.
                                    272 ;	src/text_scroller.c: 82: }
    0000C619 08               [ 3]  273 	php
    0000C61A 78               [ 2]  274 	sei
                         00000086   275 	C$text_scroller.c$78$2_0$114 ==.
                                    276 ;	src/text_scroller.c: 78: add_LCD(scanline_isr);
    0000C61B A2 C5            [ 2]  277 	ldx	#>(_scanline_isr)
    0000C61D A9 95            [ 2]  278 	lda	#(_scanline_isr)
    0000C61F 20 CF C8         [ 6]  279 	jsr	_add_LCD
    0000C622 28               [ 4]  280 	plp
                         0000008E   281 	C$text_scroller.c$89$1_0$113 ==.
                                    282 ;	src/text_scroller.c: 89: HIDE_LEFT_COLUMN;    
    0000C623 AD 25 00         [ 4]  283 	lda	_shadow_PPUMASK
    0000C626 29 F9            [ 2]  284 	and	#0xf9
    0000C628 8D 25 00         [ 4]  285 	sta	_shadow_PPUMASK
                         00000096   286 	C$text_scroller.c$90$1_0$113 ==.
                                    287 ;	src/text_scroller.c: 90: base = (uint8_t *)((uint16_t)get_bkg_xy_addr(0, SCROLL_POS) & (DEVICE_SCREEN_MAP_ENTRY_SIZE==1?0xffe0:0xffc0));
    0000C62B A9 00            [ 2]  288 	lda	#0x00
    0000C62D A2 0F            [ 2]  289 	ldx	#0x0f
    0000C62F 20 90 D7         [ 6]  290 	jsr	_get_bkg_xy_addr
    0000C632 29 E0            [ 2]  291 	and	#0xe0
    0000C634 85 34            [ 3]  292 	sta	*_main_sloc0_1_0
    0000C636 86 35            [ 3]  293 	stx	*(_main_sloc0_1_0 + 1)
    0000C638 8D 82 60         [ 4]  294 	sta	_base
    0000C63B 8E 83 60         [ 4]  295 	stx	(_base + 1)
                         000000A9   296 	C$text_scroller.c$91$1_0$113 ==.
                                    297 ;	src/text_scroller.c: 91: limit = base + (DEVICE_SCREEN_BUFFER_WIDTH * DEVICE_SCREEN_MAP_ENTRY_SIZE);
    0000C63E A6 35            [ 3]  298 	ldx	*(_main_sloc0_1_0 + 1)
    0000C640 A5 34            [ 3]  299 	lda	*_main_sloc0_1_0
    0000C642 18               [ 2]  300 	clc
    0000C643 69 20            [ 2]  301 	adc	#0x20
    0000C645 90 01            [ 4]  302 	bcc	00145$
    0000C647 E8               [ 2]  303 	inx
    0000C648                        304 00145$:
    0000C648 8D 84 60         [ 4]  305 	sta	_limit
    0000C64B 8E 85 60         [ 4]  306 	stx	(_limit + 1)
                         000000B9   307 	C$text_scroller.c$93$1_0$113 ==.
                                    308 ;	src/text_scroller.c: 93: scroller_vram_addr = base + ((DEVICE_SCREEN_X_OFFSET + DEVICE_SCREEN_WIDTH) * DEVICE_SCREEN_MAP_ENTRY_SIZE);
    0000C64E 8D 80 60         [ 4]  309 	sta	_scroller_vram_addr
    0000C651 8E 81 60         [ 4]  310 	stx	(_scroller_vram_addr + 1)
                         000000BF   311 	C$text_scroller.c$94$1_0$113 ==.
                                    312 ;	src/text_scroller.c: 94: if (scroller_vram_addr >= limit) scroller_vram_addr = base;
    0000C654 AD 80 60         [ 4]  313 	lda	_scroller_vram_addr
    0000C657 38               [ 2]  314 	sec
    0000C658 ED 84 60         [ 4]  315 	sbc	_limit
    0000C65B AD 81 60         [ 4]  316 	lda	(_scroller_vram_addr + 1)
    0000C65E ED 85 60         [ 4]  317 	sbc	(_limit + 1)
    0000C661 90 0A            [ 4]  318 	bcc	00102$
    0000C663 A5 35            [ 3]  319 	lda	*(_main_sloc0_1_0 + 1)
    0000C665 8D 81 60         [ 4]  320 	sta	(_scroller_vram_addr + 1)
    0000C668 A5 34            [ 3]  321 	lda	*_main_sloc0_1_0
    0000C66A 8D 80 60         [ 4]  322 	sta	_scroller_vram_addr
    0000C66D                        323 00102$:
                         000000D8   324 	C$text_scroller.c$96$1_0$113 ==.
                                    325 ;	src/text_scroller.c: 96: set_vram_byte(scroller_vram_addr, *scroller_next_char - 0x20);
    0000C66D AD 03 60         [ 4]  326 	lda	_scroller_next_char
    0000C670 85 4D            [ 3]  327 	sta	*(DPTR+0)
    0000C672 AD 04 60         [ 4]  328 	lda	(_scroller_next_char + 1)
    0000C675 85 4E            [ 3]  329 	sta	*(DPTR+1)
    0000C677 A0 00            [ 2]  330 	ldy	#0x00
    0000C679 B1 4D            [ 6]  331 	lda	[DPTR],y
    0000C67B 38               [ 2]  332 	sec
    0000C67C E9 20            [ 2]  333 	sbc	#0x20
    0000C67E 8D 10 00         [ 4]  334 	sta	_set_vram_byte_PARM_2
    0000C681 AE 81 60         [ 4]  335 	ldx	(_scroller_vram_addr + 1)
    0000C684 AD 80 60         [ 4]  336 	lda	_scroller_vram_addr
    0000C687 20 AB D0         [ 6]  337 	jsr	_set_vram_byte
                         000000F5   338 	C$text_scroller.c$98$1_0$113 ==.
                                    339 ;	src/text_scroller.c: 98: while (1) {
    0000C68A                        340 00110$:
                         000000F5   341 	C$text_scroller.c$99$2_0$115 ==.
                                    342 ;	src/text_scroller.c: 99: scroller_x++;
    0000C68A EE 02 60         [ 6]  343 	inc	_scroller_x
                         000000F8   344 	C$text_scroller.c$100$2_0$115 ==.
                                    345 ;	src/text_scroller.c: 100: if ((scroller_x & 0x07) == 0) {
    0000C68D AD 02 60         [ 4]  346 	lda	_scroller_x
    0000C690 29 07            [ 2]  347 	and	#0x07
    0000C692 D0 62            [ 4]  348 	bne	00108$
                         000000FF   349 	C$text_scroller.c$102$3_0$116 ==.
                                    350 ;	src/text_scroller.c: 102: scroller_next_char++;
    0000C694 EE 03 60         [ 6]  351 	inc	_scroller_next_char
    0000C697 D0 03            [ 4]  352 	bne	00148$
    0000C699 EE 04 60         [ 6]  353 	inc	(_scroller_next_char + 1)
    0000C69C                        354 00148$:
                         00000107   355 	C$text_scroller.c$103$3_0$116 ==.
                                    356 ;	src/text_scroller.c: 103: if (*scroller_next_char == 0) scroller_next_char = scroller_text;
    0000C69C AD 03 60         [ 4]  357 	lda	_scroller_next_char
    0000C69F 85 4D            [ 3]  358 	sta	*(DPTR+0)
    0000C6A1 AD 04 60         [ 4]  359 	lda	(_scroller_next_char + 1)
    0000C6A4 85 4E            [ 3]  360 	sta	*(DPTR+1)
    0000C6A6 A0 00            [ 2]  361 	ldy	#0x00
    0000C6A8 B1 4D            [ 6]  362 	lda	[DPTR],y
    0000C6AA D0 0A            [ 4]  363 	bne	00104$
    0000C6AC A9 C7            [ 2]  364 	lda	#>_scroller_text
    0000C6AE 8D 04 60         [ 4]  365 	sta	(_scroller_next_char + 1)
    0000C6B1 A9 18            [ 2]  366 	lda	#_scroller_text
    0000C6B3 8D 03 60         [ 4]  367 	sta	_scroller_next_char
    0000C6B6                        368 00104$:
                         00000121   369 	C$text_scroller.c$106$3_0$116 ==.
                                    370 ;	src/text_scroller.c: 106: scroller_vram_addr += DEVICE_SCREEN_MAP_ENTRY_SIZE;
    0000C6B6 EE 80 60         [ 6]  371 	inc	_scroller_vram_addr
    0000C6B9 D0 03            [ 4]  372 	bne	00151$
    0000C6BB EE 81 60         [ 6]  373 	inc	(_scroller_vram_addr + 1)
    0000C6BE                        374 00151$:
                         00000129   375 	C$text_scroller.c$107$3_0$116 ==.
                                    376 ;	src/text_scroller.c: 107: if (scroller_vram_addr >= limit) scroller_vram_addr = base;
    0000C6BE AD 80 60         [ 4]  377 	lda	_scroller_vram_addr
    0000C6C1 38               [ 2]  378 	sec
    0000C6C2 ED 84 60         [ 4]  379 	sbc	_limit
    0000C6C5 AD 81 60         [ 4]  380 	lda	(_scroller_vram_addr + 1)
    0000C6C8 ED 85 60         [ 4]  381 	sbc	(_limit + 1)
    0000C6CB 90 0C            [ 4]  382 	bcc	00106$
    0000C6CD AD 83 60         [ 4]  383 	lda	(_base + 1)
    0000C6D0 8D 81 60         [ 4]  384 	sta	(_scroller_vram_addr + 1)
    0000C6D3 AD 82 60         [ 4]  385 	lda	_base
    0000C6D6 8D 80 60         [ 4]  386 	sta	_scroller_vram_addr
    0000C6D9                        387 00106$:
                         00000144   388 	C$text_scroller.c$110$3_0$116 ==.
                                    389 ;	src/text_scroller.c: 110: set_vram_byte(scroller_vram_addr, *scroller_next_char - 0x20);
    0000C6D9 AD 03 60         [ 4]  390 	lda	_scroller_next_char
    0000C6DC 85 4D            [ 3]  391 	sta	*(DPTR+0)
    0000C6DE AD 04 60         [ 4]  392 	lda	(_scroller_next_char + 1)
    0000C6E1 85 4E            [ 3]  393 	sta	*(DPTR+1)
    0000C6E3 A0 00            [ 2]  394 	ldy	#0x00
    0000C6E5 B1 4D            [ 6]  395 	lda	[DPTR],y
    0000C6E7 38               [ 2]  396 	sec
    0000C6E8 E9 20            [ 2]  397 	sbc	#0x20
    0000C6EA 8D 10 00         [ 4]  398 	sta	_set_vram_byte_PARM_2
    0000C6ED AE 81 60         [ 4]  399 	ldx	(_scroller_vram_addr + 1)
    0000C6F0 AD 80 60         [ 4]  400 	lda	_scroller_vram_addr
    0000C6F3 20 AB D0         [ 6]  401 	jsr	_set_vram_byte
    0000C6F6                        402 00108$:
                                    403 ;	c:\geeky\gbdk-2020\build\gbdk\include\nes\nes.h: 939: bkg_scroll_x = x, bkg_scroll_y = y;
    0000C6F6 A2 00            [ 2]  404 	ldx	#0x00
    0000C6F8 8E 28 00         [ 4]  405 	stx	_bkg_scroll_x
    0000C6FB 8E 29 00         [ 4]  406 	stx	_bkg_scroll_y
                         00000169   407 	C$text_scroller.c$115$2_0$115 ==.
                                    408 ;	src/text_scroller.c: 115: _lcd_scanline = 0;
    0000C6FE 8E 44 00         [ 4]  409 	stx	__lcd_scanline
                         0000016C   410 	C$text_scroller.c$117$2_0$115 ==.
                                    411 ;	src/text_scroller.c: 117: vsync();        
    0000C701 20 68 C3         [ 6]  412 	jsr	_vsync
    0000C704 4C 8A C6         [ 3]  413 	jmp	00110$
                         00000172   414 	C$text_scroller.c$119$1_0$113 ==.
                                    415 ;	src/text_scroller.c: 119: }
                         00000172   416 	C$text_scroller.c$119$1_0$113 ==.
                         00000172   417 	XG$main$0$0 ==.
    0000C707 60               [ 6]  418 	rts
                                    419 	.area _CODE
                         00000173   420 G$scanline_offsets_tbl$0_0$0 == .
    0000C708                        421 _scanline_offsets_tbl:
    0000C708 00                     422 	.db #0x00	; 0
    0000C709 01                     423 	.db #0x01	; 1
    0000C70A 02                     424 	.db #0x02	; 2
    0000C70B 03                     425 	.db #0x03	; 3
    0000C70C 03                     426 	.db #0x03	; 3
    0000C70D 02                     427 	.db #0x02	; 2
    0000C70E 01                     428 	.db #0x01	; 1
    0000C70F 00                     429 	.db #0x00	; 0
    0000C710 00                     430 	.db #0x00	; 0
    0000C711 01                     431 	.db #0x01	; 1
    0000C712 02                     432 	.db #0x02	; 2
    0000C713 03                     433 	.db #0x03	; 3
    0000C714 03                     434 	.db #0x03	; 3
    0000C715 02                     435 	.db #0x02	; 2
    0000C716 01                     436 	.db #0x01	; 1
    0000C717 00                     437 	.db #0x00	; 0
                         00000183   438 G$scroller_text$0_0$0 == .
    0000C718                        439 _scroller_text:
    0000C718 54 68 69 73 20 69 73   440 	.ascii "This is a text scroller demo for GBDK-2020. You can use idea"
             20 61 20 74 65 78 74
             20 73 63 72 6F 6C 6C
             65 72 20 64 65 6D 6F
             20 66 6F 72 20 47 42
             44 4B 2D 32 30 32 30
             2E 20 59 6F 75 20 63
             61 6E 20 75 73 65 20
             69 64 65 61
    0000C754 73 2C 20 74 68 61 74   441 	.ascii "s, that are shown in this demo, to make different parallax e"
             20 61 72 65 20 73 68
             6F 77 6E 20 69 6E 20
             74 68 69 73 20 64 65
             6D 6F 2C 20 74 6F 20
             6D 61 6B 65 20 64 69
             66 66 65 72 65 6E 74
             20 70 61 72 61 6C 6C
             61 78 20 65
    0000C790 66 66 65 63 74 73 2C   442 	.ascii "ffects, scrolling of tilemaps which are larger than 32x32 ti"
             20 73 63 72 6F 6C 6C
             69 6E 67 20 6F 66 20
             74 69 6C 65 6D 61 70
             73 20 77 68 69 63 68
             20 61 72 65 20 6C 61
             72 67 65 72 20 74 68
             61 6E 20 33 32 78 33
             32 20 74 69
    0000C7CC 6C 65 73 20 61 6E 64   443 	.ascii "les and TEXT SCROLLERS, of course! Need to write something e"
             20 54 45 58 54 20 53
             43 52 4F 4C 4C 45 52
             53 2C 20 6F 66 20 63
             6F 75 72 73 65 21 20
             4E 65 65 64 20 74 6F
             20 77 72 69 74 65 20
             73 6F 6D 65 74 68 69
             6E 67 20 65
    0000C808 6C 73 65 20 74 6F 20   444 	.ascii "lse to make this text longer than 256 characters. The quick "
             6D 61 6B 65 20 74 68
             69 73 20 74 65 78 74
             20 6C 6F 6E 67 65 72
             20 74 68 61 6E 20 32
             35 36 20 63 68 61 72
             61 63 74 65 72 73 2E
             20 54 68 65 20 71 75
             69 63 6B 20
    0000C844 72 65 64 20 66 6F 78   445 	.ascii "red fox jumps over the lazy brown dog. 0123456789.          "
             20 6A 75 6D 70 73 20
             6F 76 65 72 20 74 68
             65 20 6C 61 7A 79 20
             62 72 6F 77 6E 20 64
             6F 67 2E 20 30 31 32
             33 34 35 36 37 38 39
             2E 20 20 20 20 20 20
             20 20 20 20
    0000C880 00                     446 	.db 0x00
                         000002EC   447 Ftext_scroller$__str_0$0_0$0 == .
    0000C881                        448 ___str_0:
    0000C881 20 53 63 72 6F 6C 6C   449 	.ascii " Scrolling %d chars"
             69 6E 67 20 25 64 20
             63 68 61 72 73
    0000C894 00                     450 	.db 0x00
                                    451 	.area _XINIT
                         00000000   452 Ftext_scroller$__xinit_scanline_offsets$0_0$0 == .
    0000DB76                        453 __xinit__scanline_offsets:
    0000DB76 08 C7                  454 	.dw _scanline_offsets_tbl
                         00000002   455 Ftext_scroller$__xinit_scroller_x$0_0$0 == .
    0000DB78                        456 __xinit__scroller_x:
    0000DB78 00                     457 	.db #0x00	; 0
                         00000003   458 Ftext_scroller$__xinit_scroller_next_char$0_0$0 == .
    0000DB79                        459 __xinit__scroller_next_char:
    0000DB79 18 C7                  460 	.dw _scroller_text
                                    461 	.area _CABS    (ABS)
