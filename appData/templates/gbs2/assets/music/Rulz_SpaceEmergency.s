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
; 00 : Rulz_SpaceEmergency
	.dw .music_data__song0ch0
	.dw .music_data__song0ch1
	.dw .music_data__song0ch2
	.dw .music_data__song0ch3
	.dw .music_data__song0ch4
	.dw 307,256

.globl music_data_

.music_data__instruments:
	.dw .music_data__env1,.music_data__env2,.music_data__env3,.music_data__env0 ; 00 : New instrument 1

.music_data__env0:
	.db 0x00,0xc0,0x7f,0x00,0x02
.music_data__env1:
	.db 0x00,0xcf,0x7f,0x00,0x02
.music_data__env2:
	.db 0xc0,0x7f,0x00,0x01
.music_data__env3:
	.db 0x7f,0x00,0x00

.music_data__samples:

.music_data__song0ch0:
	.db 0x46, 0x08
.music_data__song0ch0loop:
	.db 0x81, 0x80
.music_data__song0ref6:
	.db 0x50, 0xf9, 0x25, 0x26, 0x43, 0x50, 0xf9, 0x26, 0x27, 0x43, 0x50, 0xfa, 0x27, 0x28, 0x43, 0x50, 0xfa, 0x28, 0x29, 0x43
	.db 0x50, 0xfa, 0x29, 0x2a, 0x43, 0x50, 0xfa, 0x2a, 0x2b, 0x43, 0x50, 0xfb, 0x2b, 0x2c, 0x43, 0x50, 0xfb, 0x2c, 0x2d
	.db 0x41, 0x08
	.dw .music_data__song0ref6
	.db 0x48, 0x00, 0x50, 0x80, 0x25, 0x60, 0x00, 0xd9, 0x42
	.dw .music_data__song0ch0loop
.music_data__song0ch1:
.music_data__song0ch1loop:
	.db 0x81, 0x48, 0x02, 0x80, 0x25, 0x00, 0x48, 0x02, 0x25, 0x00, 0x48, 0x02, 0x25, 0x48, 0x02, 0x25, 0x00, 0x25, 0x48, 0x02
	.db 0x81, 0x00, 0x48, 0x02, 0x25, 0x00, 0x48, 0x02, 0x25, 0x48, 0x02, 0x25, 0x25, 0x48, 0x02, 0x81, 0x00, 0xdb, 0x42
	.dw .music_data__song0ch1loop
.music_data__song0ch2:
.music_data__song0ch2loop:
	.db 0xff, 0x42
	.dw .music_data__song0ch2loop
.music_data__song0ch3:
.music_data__song0ch3loop:
	.db 0x81, 0x7a, 0x80, 0x23, 0x79, 0x22, 0x78, 0x21, 0x20, 0x77, 0x1f, 0x76, 0x1e, 0x1d, 0x75, 0x1c, 0x1b, 0x74, 0x1a, 0x73
	.db 0x1a, 0x19, 0x72, 0x18, 0x71, 0x17, 0x16, 0x15, 0x81, 0x48, 0x00, 0x81, 0x00, 0xd7, 0x42
	.dw .music_data__song0ch3loop
.music_data__song0ch4:
.music_data__song0ch4loop:
	.db 0xff, 0x42
	.dw .music_data__song0ch4loop
