#include "vm.h"

#include "vm_actor.h"
#include "vm_ui.h"
#include "vm_gameboy.h"
#include "vm_music.h"
#include "vm_camera.h"
#include "vm_math.h"
#include "vm_rtc.h"
#include "vm_projectiles.h"
#include "vm_scene.h"
#include "vm_palette.h"
#include "vm_sgb.h"
#include "vm_sio.h"
#include "vm_load_save.h"
#include "vm_gbprinter.h"

// here we define all VM instructions: their handlers and parameter lengths in bytes
// this array must be nonbanked as well as STEP_VM()
#if defined(NINTENDO_NES)
    // Optimized structures-of-array for 6502 asm code

void dummy_script_cmds_func(void)
{
__asm

_script_cmds_addr_lo::
    .db #<_vm_push                          // 0x01
    .db #<_vm_pop                           // 0x02
    .db 0
    .db #<_vm_call                          // 0x04
    .db #<_vm_ret                           // 0x05
    .db #<_vm_get_far                       // 0x06
    .db #<_vm_loop                          // 0x07
    .db #<_vm_switch                        // 0x08
    .db #<_vm_jump                          // 0x09
    .db #<_vm_call_far                      // 0x0A
    .db #<_vm_ret_far                       // 0x0B
    .db 0
    .db #<_vm_invoke                        // 0x0D
    .db #<_vm_beginthread                   // 0x0E
    .db #<_vm_if                            // 0x0F
    .db #<_vm_push_value_ind                // 0x10
    .db #<_vm_push_value                    // 0x11
    .db #<_vm_reserve                       // 0x12
    .db #<_vm_set                           // 0x13
    .db #<_vm_set_const                     // 0x14
    .db 0 ; #<_vm_rpn                           // 0x15
    .db #<_vm_join                          // 0x16
    .db #<_vm_terminate                     // 0x17
    .db #<_vm_idle                          // 0x18
    .db #<_vm_get_tlocal                    // 0x19
    .db #<_vm_if_const                      // 0x1A
    .db 0
    .db 0
    .db 0
    .db 0
    .db 0
    .db 0
    .db 0
    .db 0
    .db #<_vm_init_rng                      // 0x23
    .db #<_vm_rand                          // 0x24
    .db #<_vm_lock                          // 0x25
    .db #<_vm_unlock                        // 0x26
    .db #<_vm_raise                         // 0x27
    .db #<_vm_set_indirect                  // 0x28
    .db #<_vm_get_indirect                  // 0x29
    .db #<_vm_test_terminate                // 0x2A
    .db #<_vm_poll_loaded                   // 0x2B
    .db #<_vm_push_reference                // 0x2C
    .db #<_vm_call_native                   // 0x2D
    // load/save instrunctions section
    .db #<_vm_save_peek                     // 0x2E
    .db #<_vm_save_clear                    // 0x2F

    // actor instructions section
    .db #<_vm_actor_move_to                 // 0x30
    .db #<_vm_actor_activate                // 0x31
    .db #<_vm_actor_set_dir                 // 0x32
    .db #<_vm_actor_deactivate              // 0x33
    .db #<_vm_actor_set_anim                // 0x34
    .db #<_vm_actor_set_pos                 // 0x35
    .db #<_vm_actor_emote                   // 0x36
    .db #<_vm_actor_set_bounds              // 0x37
    .db #<_vm_actor_set_spritesheet         // 0x38
    .db #<_vm_actor_replace_tile            // 0x39
    .db #<_vm_actor_get_pos                 // 0x3A
    .db 0
    .db #<_vm_actor_get_dir                 // 0x3C
    .db #<_vm_actor_set_anim_tick           // 0x3D
    .db #<_vm_actor_set_move_speed          // 0x3E
    .db #<_vm_actor_set_flags               // 0x3F

    // user interface instructions section
    .db #<_vm_load_text                     // 0x40
    .db #<_vm_display_text                  // 0x41
    .db #<_vm_overlay_setpos                // 0x42
    .db 0
    .db #<_vm_overlay_wait                  // 0x44
    .db #<_vm_overlay_move_to               // 0x45
    .db #<_vm_overlay_show                  // 0x46
    .db #<_vm_overlay_clear                 // 0x47
    .db #<_vm_choice                        // 0x48
    .db 0                                   // 0x49
    .db 0
    .db #<_vm_set_font                      // 0x4B
    .db #<_vm_overlay_set_submap_ex         // 0x4C
    .db #<_vm_overlay_scroll                // 0x4D
    .db #<_vm_overlay_set_scroll            // 0x4E
    .db #<_vm_overlay_set_submap            // 0x4F

    // gameboy features instructions section
    .db #<_vm_load_tileset                  // 0x50
    .db #<_vm_set_sprites_visible           // 0x51
    .db #<_vm_input_wait                    // 0x52
    .db #<_vm_input_attach                  // 0x53
    .db #<_vm_input_get                     // 0x54
    .db #<_vm_context_prepare               // 0x55
    .db #<_vm_overlay_set_map               // 0x56
    .db #<_vm_fade                          // 0x57
    .db #<_vm_timer_prepare                 // 0x58
    .db #<_vm_timer_set                     // 0x59
    .db #<_vm_get_tile_xy                   // 0x5A
    .db #<_vm_replace_tile                  // 0x5B
    .db #<_vm_poll                          // 0x5C
    .db #<_vm_set_sprite_mode               // 0x5D
    .db #<_vm_replace_tile_xy               // 0x5E
    .db #<_vm_input_detach                  // 0x5F

    // music and sound instructions section
    .db #<_vm_music_play                    // 0x60
    .db #<_vm_music_stop                    // 0x61
    .db #<_vm_music_mute                    // 0x62
    .db #<_vm_sound_mastervol               // 0x63
    .db 0
    .db #<_vm_music_routine                 // 0x65
    .db #<_vm_sfx_play                      // 0x66
    .db #<_vm_music_setpos                  // 0x67
    // scene stack instructions
    .db #<_vm_scene_push                    // 0x68
    .db #<_vm_scene_pop                     // 0x69
    .db #<_vm_scene_pop_all                 // 0x6A
    .db #<_vm_scene_stack_reset             // 0x6B

    // SIO transfers
    .db #<_vm_sio_set_mode                  // 0x6C
    .db #<_vm_sio_exchange                  // 0x6D
    .db 0
    .db 0

    // camera instructions section
    .db #<_vm_camera_move_to                // 0x70
    .db #<_vm_camera_set_pos                // 0x71
    .db #<_vm_timer_stop                    // 0x72
    .db #<_vm_timer_reset                   // 0x73
    .db #<_vm_actor_terminate_update        // 0x74
    .db #<_vm_actor_set_anim_frame          // 0x75
    // variable manipulation
    .db #<_vm_memset                        // 0x76
    .db #<_vm_memcpy                        // 0x77
    // RTC instructions section
    .db #<_vm_rtc_latch                         // 0x78
    .db #<_vm_rtc_get                           // 0x79
    .db #<_vm_rtc_set                           // 0x7A
    .db #<_vm_rtc_start                         // 0x7B
    // color instrunctions section
    .db #<_vm_load_palette                      // 0x7C
    .db 0
    // SGB instructions section
    .db #<_vm_sgb_transfer                      // 0x7E
    // RUMBLE
    .db #<_vm_rumble                            // 0x7F

    // projectiles instructions section
    .db #<_vm_projectile_launch                 // 0x80
    .db #<_vm_projectile_load_type              // 0x81
    .db 0

    .db #<_vm_actor_get_anim_frame              // 0x83
    .db #<_vm_actor_set_anim_set                // 0x84
    .db #<_vm_switch_text_layer                 // 0x85
    .db #<_vm_actor_get_angle                   // 0x86
    .db #<_vm_actor_set_spritesheet_by_ref      // 0x87
    .db #<_vm_actor_move_cancel                 // 0x88

    // trigonometry instructions section
    .db #<_vm_sin_scale                         // 0x89
    .db #<_vm_cos_scale                         // 0x8A

    .db #<_vm_set_text_sound                    // 0x8B

    // GBPrinter functions
    .db #<_vm_print_detect                      // 0x8C
    .db #<_vm_print_overlay                     // 0x8D

    .db #<_vm_actor_begin_update                // 0x8E        

_script_cmds_addr_hi::
    .db #>_vm_push                          // 0x01
    .db #>_vm_pop                           // 0x02
    .db 0
    .db #>_vm_call                          // 0x04
    .db #>_vm_ret                           // 0x05
    .db #>_vm_get_far                       // 0x06
    .db #>_vm_loop                          // 0x07
    .db #>_vm_switch                        // 0x08
    .db #>_vm_jump                          // 0x09
    .db #>_vm_call_far                      // 0x0A
    .db #>_vm_ret_far                       // 0x0B
    .db 0
    .db #>_vm_invoke                        // 0x0D
    .db #>_vm_beginthread                   // 0x0E
    .db #>_vm_if                            // 0x0F
    .db #>_vm_push_value_ind                // 0x10
    .db #>_vm_push_value                    // 0x11
    .db #>_vm_reserve                       // 0x12
    .db #>_vm_set                           // 0x13
    .db #>_vm_set_const                     // 0x14
    .db 0 ; #>_vm_rpn                           // 0x15
    .db #>_vm_join                          // 0x16
    .db #>_vm_terminate                     // 0x17
    .db #>_vm_idle                          // 0x18
    .db #>_vm_get_tlocal                    // 0x19
    .db #>_vm_if_const                      // 0x1A
    .db 0
    .db 0
    .db 0
    .db 0
    .db 0
    .db 0
    .db 0
    .db 0
    .db #>_vm_init_rng                      // 0x23
    .db #>_vm_rand                          // 0x24
    .db #>_vm_lock                          // 0x25
    .db #>_vm_unlock                        // 0x26
    .db #>_vm_raise                         // 0x27
    .db #>_vm_set_indirect                  // 0x28
    .db #>_vm_get_indirect                  // 0x29
    .db #>_vm_test_terminate                // 0x2A
    .db #>_vm_poll_loaded                   // 0x2B
    .db #>_vm_push_reference                // 0x2C
    .db #>_vm_call_native                   // 0x2D
    // load/save instrunctions section
    .db #>_vm_save_peek                     // 0x2E
    .db #>_vm_save_clear                    // 0x2F

    // actor instructions section
    .db #>_vm_actor_move_to                 // 0x30
    .db #>_vm_actor_activate                // 0x31
    .db #>_vm_actor_set_dir                 // 0x32
    .db #>_vm_actor_deactivate              // 0x33
    .db #>_vm_actor_set_anim                // 0x34
    .db #>_vm_actor_set_pos                 // 0x35
    .db #>_vm_actor_emote                   // 0x36
    .db #>_vm_actor_set_bounds              // 0x37
    .db #>_vm_actor_set_spritesheet         // 0x38
    .db #>_vm_actor_replace_tile            // 0x39
    .db #>_vm_actor_get_pos                 // 0x3A
    .db 0
    .db #>_vm_actor_get_dir                 // 0x3C
    .db #>_vm_actor_set_anim_tick           // 0x3D
    .db #>_vm_actor_set_move_speed          // 0x3E
    .db #>_vm_actor_set_flags               // 0x3F

    // user interface instructions section
    .db #>_vm_load_text                     // 0x40
    .db #>_vm_display_text                  // 0x41
    .db #>_vm_overlay_setpos                // 0x42
    .db 0
    .db #>_vm_overlay_wait                  // 0x44
    .db #>_vm_overlay_move_to               // 0x45
    .db #>_vm_overlay_show                  // 0x46
    .db #>_vm_overlay_clear                 // 0x47
    .db #>_vm_choice                        // 0x48
    .db 0                                   // 0x49
    .db 0
    .db #>_vm_set_font                      // 0x4B
    .db #>_vm_overlay_set_submap_ex         // 0x4C
    .db #>_vm_overlay_scroll                // 0x4D
    .db #>_vm_overlay_set_scroll            // 0x4E
    .db #>_vm_overlay_set_submap            // 0x4F

    // gameboy features instructions section
    .db #>_vm_load_tileset                  // 0x50
    .db #>_vm_set_sprites_visible           // 0x51
    .db #>_vm_input_wait                    // 0x52
    .db #>_vm_input_attach                  // 0x53
    .db #>_vm_input_get                     // 0x54
    .db #>_vm_context_prepare               // 0x55
    .db #>_vm_overlay_set_map               // 0x56
    .db #>_vm_fade                          // 0x57
    .db #>_vm_timer_prepare                 // 0x58
    .db #>_vm_timer_set                     // 0x59
    .db #>_vm_get_tile_xy                   // 0x5A
    .db #>_vm_replace_tile                  // 0x5B
    .db #>_vm_poll                          // 0x5C
    .db #>_vm_set_sprite_mode               // 0x5D
    .db #>_vm_replace_tile_xy               // 0x5E
    .db #>_vm_input_detach                  // 0x5F

    // music and sound instructions section
    .db #>_vm_music_play                    // 0x60
    .db #>_vm_music_stop                    // 0x61
    .db #>_vm_music_mute                    // 0x62
    .db #>_vm_sound_mastervol               // 0x63
    .db 0
    .db #>_vm_music_routine                 // 0x65
    .db #>_vm_sfx_play                      // 0x66
    .db #>_vm_music_setpos                  // 0x67
    // scene stack instructions
    .db #>_vm_scene_push                    // 0x68
    .db #>_vm_scene_pop                     // 0x69
    .db #>_vm_scene_pop_all                 // 0x6A
    .db #>_vm_scene_stack_reset             // 0x6B

    // SIO transfers
    .db #>_vm_sio_set_mode                  // 0x6C
    .db #>_vm_sio_exchange                  // 0x6D
    .db 0
    .db 0

    // camera instructions section
    .db #>_vm_camera_move_to                // 0x70
    .db #>_vm_camera_set_pos                // 0x71
    .db #>_vm_timer_stop                    // 0x72
    .db #>_vm_timer_reset                   // 0x73
    .db #>_vm_actor_terminate_update        // 0x74
    .db #>_vm_actor_set_anim_frame          // 0x75
    // variable manipulation
    .db #>_vm_memset                        // 0x76
    .db #>_vm_memcpy                        // 0x77
    // RTC instructions section
    .db #>_vm_rtc_latch                         // 0x78
    .db #>_vm_rtc_get                           // 0x79
    .db #>_vm_rtc_set                           // 0x7A
    .db #>_vm_rtc_start                         // 0x7B
    // color instrunctions section
    .db #>_vm_load_palette                      // 0x7C
    .db 0
    // SGB instructions section
    .db #>_vm_sgb_transfer                      // 0x7E
    // RUMBLE
    .db #>_vm_rumble                            // 0x7F

    // projectiles instructions section
    .db #>_vm_projectile_launch                 // 0x80
    .db #>_vm_projectile_load_type              // 0x81
    .db 0

    .db #>_vm_actor_get_anim_frame              // 0x83
    .db #>_vm_actor_set_anim_set                // 0x84
    .db #>_vm_switch_text_layer                 // 0x85
    .db #>_vm_actor_get_angle                   // 0x86
    .db #>_vm_actor_set_spritesheet_by_ref      // 0x87
    .db #>_vm_actor_move_cancel                 // 0x88

    // trigonometry instructions section
    .db #>_vm_sin_scale                         // 0x89
    .db #>_vm_cos_scale                         // 0x8A

    .db #>_vm_set_text_sound                    // 0x8B

    // GBPrinter functions
    .db #>_vm_print_detect                      // 0x8C
    .db #>_vm_print_overlay                     // 0x8D

    .db #>_vm_actor_begin_update                // 0x8E        
__endasm;
}

const uint8_t script_cmds_bank[] = {
    // system instructions section
    {BANK(VM_MAIN)},      // 0x01
    {BANK(VM_MAIN)},      // 0x02
    {0},                  
    {BANK(VM_MAIN)},      // 0x04
    {BANK(VM_MAIN)},      // 0x05
    {BANK(VM_MAIN)},      // 0x06
    {BANK(VM_MAIN)},      // 0x07
    {BANK(VM_MAIN)},      // 0x08
    {BANK(VM_MAIN)},      // 0x09
    {BANK(VM_MAIN)},      // 0x0A
    {BANK(VM_MAIN)},      // 0x0B
    {0},                  
    {BANK(VM_MAIN)},      // 0x0D
    {BANK(VM_MAIN)},      // 0x0E
    {BANK(VM_MAIN)},      // 0x0F
    {BANK(VM_MAIN)},      // 0x10
    {BANK(VM_MAIN)},      // 0x11
    {BANK(VM_MAIN)},      // 0x12
    {BANK(VM_MAIN)},      // 0x13
    {BANK(VM_MAIN)},      // 0x14
    {BANK(VM_MAIN)},      // 0x15
    {BANK(VM_MAIN)},      // 0x16
    {BANK(VM_MAIN)},      // 0x17
    {BANK(VM_MAIN)},      // 0x18
    {BANK(VM_MAIN)},      // 0x19
    {BANK(VM_MAIN)},      // 0x1A
    {0},                  
    {0},                  
    {0},                  
    {0},                  
    {0},                  
    {0},                  
    {0},                  
    {0},                  
    {BANK(VM_MAIN)},      // 0x23
    {BANK(VM_MAIN)},      // 0x24
    {BANK(VM_MAIN)},      // 0x25
    {BANK(VM_MAIN)},      // 0x26
    {BANK(VM_MAIN)},      // 0x27
    {BANK(VM_MAIN)},      // 0x28
    {BANK(VM_MAIN)},      // 0x29
    {BANK(VM_MAIN)},      // 0x2A
    {BANK(VM_MAIN)},      // 0x2B
    {BANK(VM_MAIN)},      // 0x2C
    {BANK(VM_MAIN)},      // 0x2D
    // load/save instrunctions section
    {BANK(VM_LOAD_SAVE)}, // 0x2E
    {BANK(VM_LOAD_SAVE)}, // 0x2F
    
    // actor instructions section
    {BANK(VM_ACTOR)},     // 0x30
    {BANK(VM_ACTOR)},     // 0x31
    {BANK(VM_ACTOR)},     // 0x32
    {BANK(VM_ACTOR)},     // 0x33
    {BANK(VM_ACTOR)},     // 0x34
    {BANK(VM_ACTOR)},     // 0x35
    {BANK(VM_ACTOR)},     // 0x36
    {BANK(VM_ACTOR)},     // 0x37
    {BANK(VM_ACTOR)},     // 0x38
    {BANK(VM_ACTOR)},     // 0x39
    {BANK(VM_ACTOR)},     // 0x3A
    {0},                  
    {BANK(VM_ACTOR)},     // 0x3C
    {BANK(VM_ACTOR)},     // 0x3D
    {BANK(VM_ACTOR)},     // 0x3E
    {BANK(VM_ACTOR)},     // 0x3F
    
    // user interface instructions section
    {BANK(VM_UI)},        // 0x40
    {BANK(VM_UI)},        // 0x41
    {BANK(VM_UI)},        // 0x42
    {0},                  
    {BANK(VM_UI)},        // 0x44
    {BANK(VM_UI)},        // 0x45
    {BANK(VM_UI)},        // 0x46
    {BANK(VM_UI)},        // 0x47
    {BANK(VM_UI)},        // 0x48
    {BANK(VM_GAMEBOY)},   // 0x49
    {0},                  
    {BANK(VM_UI)},        // 0x4B
    {BANK(VM_UI)},        // 0x4C
    {BANK(VM_UI)},        // 0x4D
    {BANK(VM_UI)},        // 0x4E
    {BANK(VM_UI)},        // 0x4F
    
    // gameboy features instructions section
    {BANK(VM_GAMEBOY)},   // 0x50
    {BANK(VM_GAMEBOY)},   // 0x51
    {BANK(VM_GAMEBOY)},   // 0x52
    {BANK(VM_GAMEBOY)},   // 0x53
    {BANK(VM_GAMEBOY)},   // 0x54
    {BANK(VM_GAMEBOY)},   // 0x55
    {BANK(VM_UI)},        // 0x56
    {BANK(VM_GAMEBOY)},   // 0x57
    {BANK(VM_GAMEBOY)},   // 0x58
    {BANK(VM_GAMEBOY)},   // 0x59
    {BANK(VM_GAMEBOY)},   // 0x5A
    {BANK(VM_GAMEBOY)},   // 0x5B
    {BANK(VM_GAMEBOY)},   // 0x5C
    {BANK(VM_GAMEBOY)},   // 0x5D
    {BANK(VM_GAMEBOY)},   // 0x5E
    {BANK(VM_GAMEBOY)},   // 0x5F
    
    // music and sound instructions section
    {BANK(VM_MUSIC)},     // 0x60
    {BANK(VM_MUSIC)},     // 0x61
    {BANK(VM_MUSIC)},     // 0x62
    {BANK(VM_MUSIC)},     // 0x63
    {0},                  
    {BANK(VM_MUSIC)},     // 0x65
    {BANK(VM_MUSIC)},     // 0x66
    {BANK(VM_MUSIC)},     // 0x67
    // scene stack instructions
    {BANK(VM_SCENE)},     // 0x68
    {BANK(VM_SCENE)},     // 0x69
    {BANK(VM_SCENE)},     // 0x6A
    {BANK(VM_SCENE)},     // 0x6B
    
    // SIO transfers
    {BANK(VM_SIO)},       // 0x6C
    {BANK(VM_SIO)},       // 0x6D
    {0},                  
    {0},                  
    
    // camera instructions section
    {BANK(VM_CAMERA)},    // 0x70
    {BANK(VM_CAMERA)},    // 0x71
    {BANK(VM_GAMEBOY)},   // 0x72
    {BANK(VM_GAMEBOY)},   // 0x73
    {BANK(VM_ACTOR)},     // 0x74
    {BANK(VM_ACTOR)},     // 0x75
    // variable manipulation
    {BANK(VM_MAIN)},      // 0x76
    {BANK(VM_MAIN)},      // 0x77
    // RTC instructions section
    {BANK(VM_RTC)},       // 0x78
    {BANK(VM_RTC)},       // 0x79
    {BANK(VM_RTC)},       // 0x7A
    {BANK(VM_RTC)},       // 0x7B
    // color instrunctions section
    {BANK(VM_PALETTE)},   // 0x7C
    {0},                  
    // SGB instructions section
    {BANK(VM_SGB)},       // 0x7E
    // RUMBLE
    {BANK(VM_GAMEBOY)},   // 0x7F
    
    // projectiles instructions section
    {BANK(VM_PROJECTILE)},    // 0x80
    {BANK(VM_PROJECTILE)},    // 0x81
    {0},                      
    
    {BANK(VM_ACTOR)},         // 0x83
    {BANK(VM_ACTOR)},         // 0x84
    {BANK(VM_UI)},            // 0x85
    {BANK(VM_ACTOR)},         // 0x86
    {BANK(VM_ACTOR)},         // 0x87
    {BANK(VM_ACTOR)},         //0x88
    
    // trigonometry instructions section
    {BANK(VM_MATH)},          // 0x89
    {BANK(VM_MATH)},          // 0x8A
    
    {BANK(VM_UI)},            // 0x8B
    
    // GBPrinter functions
    {BANK(VM_GBPRINTER)},     // 0x8C
    {BANK(VM_GBPRINTER)},     // 0x8D
    
    {BANK(VM_ACTOR)}          // 0x8E
};

const uint8_t script_cmds_nargs[] = {
    // system instructions section
    2, // 0x01
    1, // 0x02
    0,
    2, // 0x04
    1, // 0x05
    6, // 0x06
    5, // 0x07
    4, // 0x08
    2, // 0x09
    3, // 0x0A
    1, // 0x0B
    0,
    6, // 0x0D
    6, // 0x0E
    8, // 0x0F
    2, // 0x10
    2, // 0x11
    1, // 0x12
    4, // 0x13
    4, // 0x14
    0, // 0x15
    2, // 0x16
    2, // 0x17
    0, // 0x18
    4, // 0x19
    8, // 0x1A
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    2, // 0x23
    6, // 0x24
    0, // 0x25
    0, // 0x26
    2, // 0x27
    4, // 0x28
    4, // 0x29
    1, // 0x2A
    2, // 0x2B
    2, // 0x2C
    3, // 0x2D
    // load/save instrunctions section
    9, // 0x2E
    1, // 0x2F

    // actor instructions section
    2, // 0x30
    2, // 0x31
    3, // 0x32
    2, // 0x33
    4, // 0x34
    2, // 0x35
    5, // 0x36
    6, // 0x37
    5, // 0x38
    8, // 0x39
    2, // 0x3A
    0,
    4, // 0x3C
    3, // 0x3D
    3, // 0x3E
    4, // 0x3F

    // user interface instructions section
    1, // 0x40
    2, // 0x41
    2, // 0x42
    0,
    2, // 0x44
    3, // 0x45
    4, // 0x46
    6, // 0x47
    4, // 0x48
    5, // 0x49
    0,
    1, // 0x4B
    2, // 0x4C
    5, // 0x4D
    5, // 0x4E
    6, // 0x4F

    // gameboy features instructions section
    5, // 0x50
    1, // 0x51
    1, // 0x52
    2, // 0x53
    3, // 0x54
    4, // 0x55
    9, // 0x56
    1, // 0x57
    4, // 0x58
    2, // 0x59
    6, // 0x5A
    8, // 0x5B
    5, // 0x5C
    1, // 0x5D
    7, // 0x5E
    1, // 0x5F

    // music and sound instructions section
    3, // 0x60
    0, // 0x61
    1, // 0x62
    1, // 0x63
    0,
    4, // 0x65
    5, // 0x66
    2, // 0x67
    // scene stack instructions
    0, // 0x68
    0, // 0x69
    0, // 0x6A
    0, // 0x6B

    // SIO transfers
    1, // 0x6C
    5, // 0x6D
    0,
    0,

    // camera instructions section
    4, // 0x70
    2, // 0x71
    1, // 0x72
    1, // 0x73
    2, // 0x74
    2, // 0x75
    // variable manipulation
    6, // 0x76
    6, // 0x77
    // RTC instructions section
    0, // 0x78
    3, // 0x79
    3, // 0x7A
    1, // 0x7B
    // color instrunctions section
    2, // 0x7C
    0,
    // SGB instructions section
    0, // 0x7E
    // RUMBLE
    1, // 0x7F

    // projectiles instructions section
    3, // 0x80
    4, // 0x81
    0,

    2, // 0x83
    4, // 0x84
    1, // 0x85
    4, // 0x86
    4, // 0x87
    2, //0x88

    // trigonometry instructions section
    5, // 0x89
    5, // 0x8A

    4, // 0x8B

    // GBPrinter functions
    3, // 0x8C
    5, // 0x8D

    2  // 0x8E
};

#else
const SCRIPT_CMD script_cmds[] = {
    // system instructions section
    {vm_push,                           BANK(VM_MAIN),          2}, // 0x01
    {vm_pop,                            BANK(VM_MAIN),          1}, // 0x02
    {0, 0, 0},
    {vm_call,                           BANK(VM_MAIN),          2}, // 0x04
    {vm_ret,                            BANK(VM_MAIN),          1}, // 0x05
    {vm_get_far,                        BANK(VM_MAIN),          6}, // 0x06
    {vm_loop,                           BANK(VM_MAIN),          5}, // 0x07
    {vm_switch,                         BANK(VM_MAIN),          4}, // 0x08
    {vm_jump,                           BANK(VM_MAIN),          2}, // 0x09
    {vm_call_far,                       BANK(VM_MAIN),          3}, // 0x0A
    {vm_ret_far,                        BANK(VM_MAIN),          1}, // 0x0B
    {0, 0, 0},
    {vm_invoke,                         BANK(VM_MAIN),          6}, // 0x0D
    {vm_beginthread,                    BANK(VM_MAIN),          6}, // 0x0E
    {vm_if,                             BANK(VM_MAIN),          8}, // 0x0F
    {vm_push_value_ind,                 BANK(VM_MAIN),          2}, // 0x10
    {vm_push_value,                     BANK(VM_MAIN),          2}, // 0x11
    {vm_reserve,                        BANK(VM_MAIN),          1}, // 0x12
    {vm_set,                            BANK(VM_MAIN),          4}, // 0x13
    {vm_set_const,                      BANK(VM_MAIN),          4}, // 0x14
    {0, /*vm_rpn,*/                     BANK(VM_MAIN),          0}, // 0x15
    {vm_join,                           BANK(VM_MAIN),          2}, // 0x16
    {vm_terminate,                      BANK(VM_MAIN),          2}, // 0x17
    {vm_idle,                           BANK(VM_MAIN),          0}, // 0x18
    {vm_get_tlocal,                     BANK(VM_MAIN),          4}, // 0x19
    {vm_if_const,                       BANK(VM_MAIN),          8}, // 0x1A
    {0, 0, 0},
    {0, 0, 0},
    {0, 0, 0},
    {0, 0, 0},
    {0, 0, 0},
    {0, 0, 0},
    {0, 0, 0},
    {0, 0, 0},
    {vm_init_rng,                       BANK(VM_MAIN),          2}, // 0x23
    {vm_rand,                           BANK(VM_MAIN),          6}, // 0x24
    {vm_lock,                           BANK(VM_MAIN),          0}, // 0x25
    {vm_unlock,                         BANK(VM_MAIN),          0}, // 0x26
    {vm_raise,                          BANK(VM_MAIN),          2}, // 0x27
    {vm_set_indirect,                   BANK(VM_MAIN),          4}, // 0x28
    {vm_get_indirect,                   BANK(VM_MAIN),          4}, // 0x29
    {vm_test_terminate,                 BANK(VM_MAIN),          1}, // 0x2A
    {vm_poll_loaded,                    BANK(VM_MAIN),          2}, // 0x2B
    {vm_push_reference,                 BANK(VM_MAIN),          2}, // 0x2C
    {vm_call_native,                    BANK(VM_MAIN),          3}, // 0x2D
    // load/save instrunctions section
    {vm_save_peek,                      BANK(VM_LOAD_SAVE),     9}, // 0x2E
    {vm_save_clear,                     BANK(VM_LOAD_SAVE),     1}, // 0x2F

    // actor instructions section
    {vm_actor_move_to,                  BANK(VM_ACTOR),         2}, // 0x30
    {vm_actor_activate,                 BANK(VM_ACTOR),         2}, // 0x31
    {vm_actor_set_dir,                  BANK(VM_ACTOR),         3}, // 0x32
    {vm_actor_deactivate,               BANK(VM_ACTOR),         2}, // 0x33
    {vm_actor_set_anim,                 BANK(VM_ACTOR),         4}, // 0x34
    {vm_actor_set_pos,                  BANK(VM_ACTOR),         2}, // 0x35
    {vm_actor_emote,                    BANK(VM_ACTOR),         5}, // 0x36
    {vm_actor_set_bounds,               BANK(VM_ACTOR),         6}, // 0x37
    {vm_actor_set_spritesheet,          BANK(VM_ACTOR),         5}, // 0x38
    {vm_actor_replace_tile,             BANK(VM_ACTOR),         8}, // 0x39
    {vm_actor_get_pos,                  BANK(VM_ACTOR),         2}, // 0x3A
    {0, 0, 0},
    {vm_actor_get_dir,                  BANK(VM_ACTOR),         4}, // 0x3C
    {vm_actor_set_anim_tick,            BANK(VM_ACTOR),         3}, // 0x3D
    {vm_actor_set_move_speed,           BANK(VM_ACTOR),         3}, // 0x3E
    {vm_actor_set_flags,                BANK(VM_ACTOR),         4}, // 0x3F

    // user interface instructions section
    {vm_load_text,                      BANK(VM_UI),            1}, // 0x40
    {vm_display_text,                   BANK(VM_UI),            2}, // 0x41
    {vm_overlay_setpos,                 BANK(VM_UI),            2}, // 0x42
    {0, 0, 0},
    {vm_overlay_wait,                   BANK(VM_UI),            2}, // 0x44
    {vm_overlay_move_to,                BANK(VM_UI),            3}, // 0x45
    {vm_overlay_show,                   BANK(VM_UI),            4}, // 0x46
    {vm_overlay_clear,                  BANK(VM_UI),            6}, // 0x47
    {vm_choice,                         BANK(VM_UI),            4}, // 0x48
    {0, 0, 0},
    {0, 0, 0},
    {vm_set_font,                       BANK(VM_UI),            1}, // 0x4B
    {vm_overlay_set_submap_ex,          BANK(VM_UI),            2}, // 0x4C
    {vm_overlay_scroll,                 BANK(VM_UI),            5}, // 0x4D
    {vm_overlay_set_scroll,             BANK(VM_UI),            5}, // 0x4E
    {vm_overlay_set_submap,             BANK(VM_UI),            6}, // 0x4F

    // gameboy features instructions section
    {vm_load_tileset,                   BANK(VM_GAMEBOY),       5}, // 0x50
    {vm_set_sprites_visible,            BANK(VM_GAMEBOY),       1}, // 0x51
    {vm_input_wait,                     BANK(VM_GAMEBOY),       1}, // 0x52
    {vm_input_attach,                   BANK(VM_GAMEBOY),       2}, // 0x53
    {vm_input_get,                      BANK(VM_GAMEBOY),       3}, // 0x54
    {vm_context_prepare,                BANK(VM_GAMEBOY),       4}, // 0x55
    {vm_overlay_set_map,                BANK(VM_UI),            9}, // 0x56
    {vm_fade,                           BANK(VM_GAMEBOY),       1}, // 0x57
    {vm_timer_prepare,                  BANK(VM_GAMEBOY),       4}, // 0x58
    {vm_timer_set,                      BANK(VM_GAMEBOY),       2}, // 0x59
    {vm_get_tile_xy,                    BANK(VM_GAMEBOY),       6}, // 0x5A
    {vm_replace_tile,                   BANK(VM_GAMEBOY),       8}, // 0x5B
    {vm_poll,                           BANK(VM_GAMEBOY),       5}, // 0x5C
    {vm_set_sprite_mode,                BANK(VM_GAMEBOY),       1}, // 0x5D
    {vm_replace_tile_xy,                BANK(VM_GAMEBOY),       7}, // 0x5E
    {vm_input_detach,                   BANK(VM_GAMEBOY),       1}, // 0x5F

    // music and sound instructions section
    {vm_music_play,                     BANK(VM_MUSIC),         3}, // 0x60
    {vm_music_stop,                     BANK(VM_MUSIC),         0}, // 0x61
    {vm_music_mute,                     BANK(VM_MUSIC),         1}, // 0x62
    {vm_sound_mastervol,                BANK(VM_MUSIC),         1}, // 0x63
    {0, 0, 0},
    {vm_music_routine,                  BANK(VM_MUSIC),         4}, // 0x65
    {vm_sfx_play,                       BANK(VM_MUSIC),         5}, // 0x66
    {vm_music_setpos,                   BANK(VM_MUSIC),         2}, // 0x67
    // scene stack instructions
    {vm_scene_push,                     BANK(VM_SCENE),         0}, // 0x68
    {vm_scene_pop,                      BANK(VM_SCENE),         0}, // 0x69
    {vm_scene_pop_all,                  BANK(VM_SCENE),         0}, // 0x6A
    {vm_scene_stack_reset,              BANK(VM_SCENE),         0}, // 0x6B

    // SIO transfers
    {vm_sio_set_mode,                   BANK(VM_SIO),           1}, // 0x6C
    {vm_sio_exchange,                   BANK(VM_SIO),           5}, // 0x6D
    {0, 0, 0},
    {0, 0, 0},

    // camera instructions section
    {vm_camera_move_to,                 BANK(VM_CAMERA),        4}, // 0x70
    {vm_camera_set_pos,                 BANK(VM_CAMERA),        2}, // 0x71
    {vm_timer_stop,                     BANK(VM_GAMEBOY),       1}, // 0x72
    {vm_timer_reset,                    BANK(VM_GAMEBOY),       1}, // 0x73
    {vm_actor_terminate_update,         BANK(VM_ACTOR),         2}, // 0x74
    {vm_actor_set_anim_frame,           BANK(VM_ACTOR),         2}, // 0x75
    // variable manipulation
    {vm_memset,                         BANK(VM_MAIN),          6}, // 0x76
    {vm_memcpy,                         BANK(VM_MAIN),          6}, // 0x77
    // RTC instructions section
    {vm_rtc_latch,                      BANK(VM_RTC),           0}, // 0x78
    {vm_rtc_get,                        BANK(VM_RTC),           3}, // 0x79
    {vm_rtc_set,                        BANK(VM_RTC),           3}, // 0x7A
    {vm_rtc_start,                      BANK(VM_RTC),           1}, // 0x7B
    // color instrunctions section
    {vm_load_palette,                   BANK(VM_PALETTE),       2}, // 0x7C
    {0, 0, 0},
    // SGB instructions section
    {vm_sgb_transfer,                   BANK(VM_SGB),           0}, // 0x7E
    // RUMBLE
    {vm_rumble,                         BANK(VM_GAMEBOY),       1}, // 0x7F

    // projectiles instructions section
    {vm_projectile_launch,              BANK(VM_PROJECTILE),    3}, // 0x80
    {vm_projectile_load_type,           BANK(VM_PROJECTILE),    4}, // 0x81
    {0, 0, 0},

    {vm_actor_get_anim_frame,           BANK(VM_ACTOR),         2}, // 0x83
    {vm_actor_set_anim_set,             BANK(VM_ACTOR),         4}, // 0x84
    {vm_switch_text_layer,              BANK(VM_UI),            1}, // 0x85
    {vm_actor_get_angle,                BANK(VM_ACTOR),         4}, // 0x86
    {vm_actor_set_spritesheet_by_ref,   BANK(VM_ACTOR),         4}, // 0x87
    {vm_actor_move_cancel,              BANK(VM_ACTOR),         2}, //0x88

    // trigonometry instructions section
    {vm_sin_scale,                      BANK(VM_MATH),          5}, // 0x89
    {vm_cos_scale,                      BANK(VM_MATH),          5}, // 0x8A

    {vm_set_text_sound,                 BANK(VM_UI),            4}, // 0x8B

    // GBPrinter functions
    {vm_print_detect,                   BANK(VM_GBPRINTER),     3}, // 0x8C
    {vm_print_overlay,                  BANK(VM_GBPRINTER),     5}, // 0x8D

    {vm_actor_begin_update,             BANK(VM_ACTOR),         2}  // 0x8E
};
#endif