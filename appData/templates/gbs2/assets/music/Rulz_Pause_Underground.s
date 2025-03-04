; This file is for the FamiStudio Sound Engine and was generated by FamiStudio
; Project uses FamiTracker tempo, you must set FAMISTUDIO_USE_FAMITRACKER_TEMPO = 1.
; Project uses delayed notes or cuts, you must set FAMISTUDIO_USE_FAMITRACKER_DELAYED_NOTES_OR_CUTS = 1.
; Volume track is used, you must set FAMISTUDIO_USE_VOLUME_TRACK = 1.
; Slide notes are used, you must set FAMISTUDIO_USE_SLIDE_NOTES = 1.

music_data_:
	.db 1
	.dw .music_data__instruments
.if FAMISTUDIO_CFG_C_BINDINGS
_music_data_=music_data_
.globl _music_data_
.endif

	.dw .music_data__samples-4
; 00 : Rulz_Pause_Underground
	.dw .music_data__song0ch0
	.dw .music_data__song0ch1
	.dw .music_data__song0ch2
	.dw .music_data__song0ch3
	.dw .music_data__song0ch4
	.dw 307,256

.globl music_data_

.music_data__instruments:
	.dw .music_data__env2,.music_data__env3,.music_data__env5,.music_data__env0 ; 00 : New instrument 1
	.dw .music_data__env1,.music_data__env3,.music_data__env5,.music_data__env0 ; 01 : New instrument 2

.music_data__env0:
	.db 0x00,0xc0,0x7f,0x00,0x02
.music_data__env1:
	.db 0x00,0xcc,0xcb,0x02,0xca,0xca,0xc9,0xc9,0xc8,0xc8,0xc7,0xc7,0xc6,0xc6,0xc5,0x02,0xc4,0x03,0xc3,0x03,0xc2,0xc2,0xc1,0x03,0xc0,0x00,0x18
.music_data__env2:
	.db 0x00,0xca,0x7f,0x00,0x02
.music_data__env3:
	.db 0xc0,0x7f,0x00,0x01
.music_data__env4:
	.db 0x7f,0x00,0x00
.music_data__env5:
	.db 0xc2,0x7f,0x00,0x00

.music_data__samples:

.music_data__song0ch0:
	.db 0x46, 0x08
.music_data__song0ch0loop:
	.db 0x7a, 0x80
.music_data__song0ref6:
	.db 0x1f, 0x48, 0x05, 0x81, 0x75, 0x1f, 0x48, 0x05, 0x81, 0x73, 0x1f, 0x48, 0x05, 0x81, 0x7a, 0x1a, 0x48, 0x02, 0x81, 0x1d
	.db 0x48, 0x05, 0x81, 0x75, 0x1d, 0x48, 0x05, 0x81, 0x73, 0x1d, 0x48, 0x05, 0x81, 0x7a, 0x1a, 0x48, 0x02, 0x81, 0x1f, 0x48
	.db 0x02, 0x81, 0x1a, 0x48, 0x05, 0x81, 0x75, 0x1a, 0x48, 0x05, 0x81, 0x1d, 0x48, 0x05, 0x81, 0x1d, 0x48, 0x05, 0x81, 0x73
	.db 0x1d, 0x48, 0x05, 0x81, 0x71, 0x1d, 0x48, 0x05, 0x81, 0x00, 0xc1, 0x7a, 0x1f, 0x48, 0x05, 0x81, 0x75, 0x1f, 0x48, 0x05
	.db 0x81, 0x7a, 0x1a, 0x48, 0x05, 0x81, 0x75, 0x1a, 0x48, 0x05, 0x81, 0x7a, 0x1d, 0x48, 0x05, 0x81, 0x75, 0x1d, 0x48, 0x05
	.db 0x81, 0x7a, 0x1a, 0x48, 0x05, 0x81, 0x75, 0x1a, 0x48, 0x05, 0x81, 0x7a, 0x1f, 0x48, 0x05, 0x81, 0x21, 0x81, 0x22, 0x48
	.db 0x00, 0x81, 0x19, 0x43, 0x19, 0x43, 0x19, 0x43, 0x19, 0x43, 0x19, 0x43, 0x19, 0x43, 0x19, 0x43, 0x19, 0x48, 0x00, 0x81
	.db 0x00, 0xbf
	.db 0x41, 0x36
	.dw .music_data__song0ref6
	.db 0x18, 0x43, 0x18, 0x43, 0x18, 0x43, 0x18, 0x43, 0x18, 0x43, 0x18, 0x43, 0x18, 0x43, 0x18, 0x48, 0x00, 0x81, 0x00, 0xbf
	.db 0x42
	.dw .music_data__song0ch0loop
.music_data__song0ch1:
.music_data__song0ch1loop:
.music_data__song0ref175:
	.db 0x7c, 0x82
.music_data__song0ref177:
	.db 0x50, 0x7f, 0x1f, 0x14, 0x43, 0x50, 0x6b, 0x14, 0x0f, 0x43, 0x50, 0x7c, 0x0f, 0x0a, 0x48, 0x01, 0x81, 0x00, 0x93, 0x50
	.db 0x7f, 0x1f, 0x11, 0x50, 0x7f, 0x1f, 0x14, 0x43, 0x50, 0x6b, 0x14, 0x0f, 0x43, 0x50, 0x7c, 0x0f, 0x0a, 0x48, 0x01, 0x81
	.db 0x50, 0x7f, 0x1f, 0x14, 0x43, 0x50, 0x6b, 0x14, 0x0f, 0x43, 0x50, 0x7c, 0x0f, 0x0a, 0x48, 0x01, 0x81, 0x00, 0x8d
	.db 0x41, 0x11
	.dw .music_data__song0ref177
	.db 0x41, 0x0f
	.dw .music_data__song0ref177
	.db 0x76, 0x80, 0x19, 0x43, 0x19, 0x75, 0x19, 0x43, 0x19, 0x74, 0x19, 0x43, 0x19, 0x73, 0x19, 0x81
	.db 0x41, 0x11
	.dw .music_data__song0ref175
	.db 0x41, 0x11
	.dw .music_data__song0ref177
	.db 0x41, 0x11
	.dw .music_data__song0ref177
	.db 0x41, 0x0f
	.dw .music_data__song0ref177
	.db 0x76, 0x80, 0x19, 0x43, 0x19, 0x75, 0x19, 0x43, 0x19, 0x74, 0x19, 0x43, 0x19, 0x73, 0x19, 0x81
	.db 0x41, 0x11
	.dw .music_data__song0ref175
	.db 0x42
	.dw .music_data__song0ch1loop
.music_data__song0ch2:
.music_data__song0ch2loop:
	.db 0x80
.music_data__song0ref294:
	.db 0x13, 0x8d, 0x0d, 0x8d, 0x0e, 0x89, 0x0d, 0x81, 0x48, 0x02, 0x81, 0x00, 0xcb, 0x13, 0x8d, 0x0d, 0x8d, 0x0e, 0x89, 0x11
	.db 0x81, 0x48, 0x02, 0x81, 0x00, 0xcb
	.db 0x41, 0x16
	.dw .music_data__song0ref294
	.db 0x42
	.dw .music_data__song0ch2loop
.music_data__song0ch3:
.music_data__song0ch3loop:
	.db 0x48, 0x02, 0x80
.music_data__song0ref330:
	.db 0x15, 0x00, 0x83, 0x48, 0x02, 0x1f, 0x00, 0x83, 0x48, 0x02, 0x1a, 0x00, 0x83, 0x48, 0x02, 0x1f, 0x00, 0x81, 0x48, 0x02
	.db 0x15, 0x48, 0x02, 0x15, 0x00, 0x83, 0x48, 0x02, 0x1f, 0x00, 0x83, 0x48, 0x02, 0x1a, 0x00, 0x83, 0x48, 0x02, 0x1f, 0x00
	.db 0x81, 0x48, 0x02, 0x15, 0x48, 0x02
	.db 0x41, 0x1a
	.dw .music_data__song0ref330
	.db 0x48, 0x02
	.db 0x41, 0x1a
	.dw .music_data__song0ref330
	.db 0x48, 0x02
	.db 0x41, 0x1a
	.dw .music_data__song0ref330
	.db 0x48, 0x02
	.db 0x41, 0x1a
	.dw .music_data__song0ref330
	.db 0x48, 0x02
	.db 0x41, 0x1a
	.dw .music_data__song0ref330
	.db 0x48, 0x02
	.db 0x41, 0x1a
	.dw .music_data__song0ref330
	.db 0x48, 0x02
	.db 0x41, 0x1a
	.dw .music_data__song0ref330
	.db 0x42
	.dw .music_data__song0ch3loop
.music_data__song0ch4:
.music_data__song0ch4loop:
	.db 0xff, 0xff, 0xff, 0xff, 0x42
	.dw .music_data__song0ch4loop
