; This file is for the FamiStudio Sound Engine and was generated by FamiStudio
; Project uses FamiTracker tempo, you must set FAMISTUDIO_USE_FAMITRACKER_TEMPO = 1.
; Project uses delayed notes or cuts, you must set FAMISTUDIO_USE_FAMITRACKER_DELAYED_NOTES_OR_CUTS = 1.
; Volume track is used, you must set FAMISTUDIO_USE_VOLUME_TRACK = 1.
; DPCM Delta Counter effect is used, you must set FAMISTUDIO_USE_DELTA_COUNTER = 1.

music_data_rulz_lightmood:
	.db 1
	.dw .music_data_rulz_lightmood_instruments
.if FAMISTUDIO_CFG_C_BINDINGS
_music_data_rulz_lightmood=music_data_rulz_lightmood
.globl _music_data_rulz_lightmood
.endif

	.dw .music_data_rulz_lightmood_samples-4
; 00 : Rulz_LightMood
	.dw .music_data_rulz_lightmood_song0ch0
	.dw .music_data_rulz_lightmood_song0ch1
	.dw .music_data_rulz_lightmood_song0ch2
	.dw .music_data_rulz_lightmood_song0ch3
	.dw .music_data_rulz_lightmood_song0ch4
	.dw 307,256

.globl music_data_rulz_lightmood

.music_data_rulz_lightmood_instruments:
	.dw .music_data_rulz_lightmood_env2,.music_data_rulz_lightmood_env4,.music_data_rulz_lightmood_env5,.music_data_rulz_lightmood_env0 ; 00 : New instrument 4
	.dw .music_data_rulz_lightmood_env7,.music_data_rulz_lightmood_env4,.music_data_rulz_lightmood_env5,.music_data_rulz_lightmood_env0 ; 01 : New instrument 5
	.dw .music_data_rulz_lightmood_env3,.music_data_rulz_lightmood_env4,.music_data_rulz_lightmood_env6,.music_data_rulz_lightmood_env0 ; 02 : New instrument 2
	.dw .music_data_rulz_lightmood_env1,.music_data_rulz_lightmood_env4,.music_data_rulz_lightmood_env6,.music_data_rulz_lightmood_env0 ; 03 : New instrument 1

.music_data_rulz_lightmood_env0:
	.db 0x00,0xc0,0x7f,0x00,0x02
.music_data_rulz_lightmood_env1:
	.db 0x00,0xcb,0xc9,0xc6,0xc4,0xc3,0xc2,0xc1,0xc0,0x00,0x08
.music_data_rulz_lightmood_env2:
	.db 0x00,0xcf,0x7f,0x00,0x02
.music_data_rulz_lightmood_env3:
	.db 0x00,0xca,0x7f,0x00,0x02
.music_data_rulz_lightmood_env4:
	.db 0xc0,0x7f,0x00,0x01
.music_data_rulz_lightmood_env5:
	.db 0x7f,0x00,0x00
.music_data_rulz_lightmood_env6:
	.db 0xc2,0x7f,0x00,0x00
.music_data_rulz_lightmood_env7:
	.db 0x00,0xc6,0xc5,0xc4,0xc3,0xc2,0xc1,0xc0,0x00,0x07

.music_data_rulz_lightmood_samples:

.music_data_rulz_lightmood_song0ch0:
	.db 0x46, 0x04
.music_data_rulz_lightmood_song0ch0loop:
	.db 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x42
	.dw .music_data_rulz_lightmood_song0ch0loop
.music_data_rulz_lightmood_song0ch1:
.music_data_rulz_lightmood_song0ch1loop:
.music_data_rulz_lightmood_song0ref16:
	.db 0x93, 0x71, 0x84, 0x1d, 0x72, 0x1e, 0x75, 0x1f, 0x76, 0x21, 0x77, 0x22, 0x78, 0x24, 0x86, 0x26, 0x8d, 0x24, 0xcd
	.db 0x41, 0x09
	.dw .music_data_rulz_lightmood_song0ref16
	.db 0x28, 0xcd
	.db 0x41, 0x0a
	.dw .music_data_rulz_lightmood_song0ref16
	.db 0x8d, 0x24, 0xbd
	.db 0x41, 0x09
	.dw .music_data_rulz_lightmood_song0ref16
	.db 0x28, 0x8d, 0x28, 0xbd
	.db 0x41, 0x0b
	.dw .music_data_rulz_lightmood_song0ref16
	.db 0x41, 0x09
	.dw .music_data_rulz_lightmood_song0ref16
	.db 0x1c, 0xcd
	.db 0x41, 0x0a
	.dw .music_data_rulz_lightmood_song0ref16
	.db 0x8d, 0x24, 0xbd
	.db 0x41, 0x09
	.dw .music_data_rulz_lightmood_song0ref16
	.db 0x1c, 0xa5, 0x72, 0x84, 0x24, 0x71, 0x24, 0x70, 0x24, 0x81, 0x73, 0x24, 0x72, 0x24, 0x71, 0x24, 0x81, 0x74, 0x24, 0x73
	.db 0x24, 0x72, 0x24, 0x71, 0x24, 0x48, 0x00, 0x81, 0x00, 0x8b, 0x42
	.dw .music_data_rulz_lightmood_song0ch1loop
.music_data_rulz_lightmood_song0ch2:
.music_data_rulz_lightmood_song0ch2loop:
	.db 0x86
.music_data_rulz_lightmood_song0ref105:
	.db 0x0d, 0x0d, 0x48, 0x00, 0x81, 0x00, 0x0d, 0x0d, 0x48, 0x00, 0x81, 0x00, 0x87, 0x0d, 0x0d, 0x48, 0x00, 0x81, 0x00, 0x9f
	.db 0x41, 0x0e
	.dw .music_data_rulz_lightmood_song0ref105
	.db 0x41, 0x0e
	.dw .music_data_rulz_lightmood_song0ref105
	.db 0x41, 0x0d
	.dw .music_data_rulz_lightmood_song0ref105
	.db 0x8f, 0x18, 0x85, 0x18, 0x81, 0x18, 0x81
	.db 0x41, 0x0e
	.dw .music_data_rulz_lightmood_song0ref105
	.db 0x41, 0x0e
	.dw .music_data_rulz_lightmood_song0ref105
	.db 0x41, 0x0e
	.dw .music_data_rulz_lightmood_song0ref105
	.db 0x41, 0x0d
	.dw .music_data_rulz_lightmood_song0ref105
	.db 0x87, 0x18, 0x18, 0x18, 0x18, 0x13, 0x13, 0x13, 0x13, 0x18, 0x18, 0x18, 0x18
.music_data_rulz_lightmood_song0ref166:
	.db 0x1d, 0x1d, 0x48, 0x00, 0x81, 0x00, 0x1d, 0x1d, 0x48, 0x00, 0x81, 0x00, 0x87, 0x1d, 0x1d, 0x48, 0x00, 0x81, 0x00, 0x97
	.db 0x18, 0x81, 0x18, 0x81
	.db 0x41, 0x09
	.dw .music_data_rulz_lightmood_song0ref166
	.db 0x0d, 0x0d, 0x48, 0x00, 0x81, 0x00, 0x9f
	.db 0x41, 0x09
	.dw .music_data_rulz_lightmood_song0ref166
	.db 0x0d, 0x0d, 0x48, 0x00, 0x81, 0x00, 0x97, 0x18, 0x81, 0x18, 0x81
	.db 0x41, 0x09
	.dw .music_data_rulz_lightmood_song0ref166
	.db 0x0d, 0xa5
.music_data_rulz_lightmood_song0ref219:
	.db 0x29, 0x29, 0x48, 0x00, 0x81, 0x00, 0x29, 0x29, 0x48, 0x00, 0x81, 0x00, 0x87, 0x0d, 0x0d, 0x48, 0x00, 0x81, 0x00, 0x97
	.db 0x24, 0x81, 0x24, 0x81
	.db 0x41, 0x0d
	.dw .music_data_rulz_lightmood_song0ref219
	.db 0x9f
	.db 0x41, 0x12
	.dw .music_data_rulz_lightmood_song0ref219
	.db 0x41, 0x0d
	.dw .music_data_rulz_lightmood_song0ref219
	.db 0x8f, 0x24, 0x85, 0x24, 0x81, 0x24, 0x81, 0x42
	.dw .music_data_rulz_lightmood_song0ch2loop
.music_data_rulz_lightmood_song0ch3:
.music_data_rulz_lightmood_song0ch3loop:
.music_data_rulz_lightmood_song0ref264:
	.db 0x48, 0x01, 0x80
.music_data_rulz_lightmood_song0ref267:
	.db 0x15, 0x00, 0x83, 0x48, 0x01, 0x15, 0x00, 0x83, 0x82, 0x1e, 0x85, 0x48, 0x01, 0x80, 0x15, 0x00, 0xa3, 0x48, 0x01
	.db 0x41, 0x0b
	.dw .music_data_rulz_lightmood_song0ref267
	.db 0x48, 0x01
	.db 0x41, 0x0b
	.dw .music_data_rulz_lightmood_song0ref267
	.db 0x48, 0x01
	.db 0x41, 0x0b
	.dw .music_data_rulz_lightmood_song0ref267
	.db 0x48, 0x01
	.db 0x41, 0x0b
	.dw .music_data_rulz_lightmood_song0ref267
	.db 0x48, 0x01
	.db 0x41, 0x0b
	.dw .music_data_rulz_lightmood_song0ref267
	.db 0x48, 0x01
	.db 0x41, 0x0b
	.dw .music_data_rulz_lightmood_song0ref267
	.db 0x48, 0x01
	.db 0x41, 0x0a
	.dw .music_data_rulz_lightmood_song0ref267
	.db 0x8b, 0x72, 0x82, 0x1e, 0x85, 0x73, 0x1e, 0x85, 0x75, 0x1e, 0x85
	.db 0x41, 0x0a
	.dw .music_data_rulz_lightmood_song0ref264
	.db 0x83, 0x48, 0x01, 0x15, 0x00, 0x8b, 0x82, 0x1e, 0x8d
	.db 0x41, 0x0b
	.dw .music_data_rulz_lightmood_song0ref264
	.db 0x48, 0x01
	.db 0x41, 0x0a
	.dw .music_data_rulz_lightmood_song0ref267
	.db 0x83, 0x48, 0x01, 0x15, 0x00, 0x8b, 0x82, 0x1e, 0x8d
	.db 0x41, 0x0b
	.dw .music_data_rulz_lightmood_song0ref264
	.db 0x48, 0x01
	.db 0x41, 0x0a
	.dw .music_data_rulz_lightmood_song0ref267
	.db 0x83, 0x48, 0x01, 0x15, 0x00, 0x8b, 0x82, 0x1e, 0x8d
	.db 0x41, 0x0b
	.dw .music_data_rulz_lightmood_song0ref264
	.db 0x48, 0x01
	.db 0x41, 0x0a
	.dw .music_data_rulz_lightmood_song0ref267
	.db 0x83, 0x48, 0x01, 0x15, 0x00, 0x8b, 0x82, 0x1e, 0x8d
	.db 0x41, 0x0b
	.dw .music_data_rulz_lightmood_song0ref264
	.db 0x42
	.dw .music_data_rulz_lightmood_song0ch3loop
.music_data_rulz_lightmood_song0ch4:
.music_data_rulz_lightmood_song0ch4loop:
.music_data_rulz_lightmood_song0ref400:
	.db 0x52, 0x80, 0x81, 0x52, 0xff, 0x85, 0x52, 0x80, 0x81, 0x52, 0xff, 0x8d, 0x52, 0x80, 0x81, 0x52, 0xff, 0xa5, 0x52, 0x80
	.db 0x81, 0x52, 0xff, 0x85, 0x52, 0x80, 0x81, 0x52, 0xff, 0x8d, 0x52, 0x80, 0x81, 0x52, 0xff, 0xa5
	.db 0x41, 0x0c
	.dw .music_data_rulz_lightmood_song0ref400
	.db 0x41, 0x0c
	.dw .music_data_rulz_lightmood_song0ref400
	.db 0x41, 0x0c
	.dw .music_data_rulz_lightmood_song0ref400
	.db 0x41, 0x0c
	.dw .music_data_rulz_lightmood_song0ref400
	.db 0x41, 0x09
	.dw .music_data_rulz_lightmood_song0ref400
	.db 0x52, 0xff, 0xb5, 0x52, 0x80, 0x81, 0x52, 0xff, 0x85, 0x52, 0x80, 0x81, 0x52, 0xff, 0x8d, 0x52, 0x80, 0x81, 0x52, 0xff
	.db 0x9d, 0x52, 0x80, 0x83, 0x52, 0xff, 0x83
	.db 0x41, 0x0b
	.dw .music_data_rulz_lightmood_song0ref400
	.db 0x52, 0xff, 0x9d, 0x52, 0x80, 0x83, 0x52, 0xff, 0x83, 0x52, 0x80, 0x81, 0x52, 0xff, 0x85, 0x52, 0x80, 0x81, 0x52, 0xff
	.db 0x8d, 0x52, 0x80, 0x81, 0x52, 0xff, 0xa5, 0x42
	.dw .music_data_rulz_lightmood_song0ch4loop
