#include <pokeagb/pokeagb.h>
#include "battle_text/battle_textbox_gfx.h"
#include "battle_data/pkmn_bank_stats.h"
#include "battle_slide_in_data/battle_obj_sliding.h"
extern void CpuFastSet(void*, void*, u32);

/* This vblank overlaps the tilemap from show_message with the battle box */
void vblank_cb_merge_tbox()
{
    gpu_sprites_upload();
    copy_queue_process();
    gpu_pal_upload();
    u16 i;
    u8 **bg0_map = (u8**)0x030008EC;
    u8 *dst = (u8 *)(*bg0_map);
    u8 *src = (u8 *)battle_textboxMap;//(u32 *)0x0600F800;
    for (i = 0; i < 2048; i++) {
    // only merge if there is no text on this tile
        if (!*(dst + i))
            *(dst + i) = *(src + i);
	}
}

void vblank_cb_merge_tbox_sliding()
{
    gpu_sprites_upload();
    copy_queue_process();
    gpu_pal_upload();
    u16 i;
    u8 **bg0_map = (u8**)0x030008EC;
    u8 *dst = (u8 *)(*bg0_map);
    u8 *src = (u8 *)battle_textboxMap;//(u32 *)0x0600F800;
    for (i = 0; i < 2048; i++) {
    // only merge if there is no text on this tile
        if (!*(dst + i))
            *(dst + i) = *(src + i);
	}
    if (gMain.state == 7) {
        bs_env_windows->bot_side -= 3;
        bs_env_windows->top_side += 3;
        bs_env_windows->wintop -= 1;
        bs_env_windows->winbot += 1;
    }
}

/* This vblank overlaps the tilemap from move selection with the battle box */
void vblank_cb_merge_move_select()
{
    gpu_sprites_upload();
    copy_queue_process();
    gpu_pal_upload();
    u16 i;
    u8 **bg0_map = (u8**)0x030008EC;
    u8 *dst = (u8 *)(*bg0_map);
    u8 *src = (u8 *)battle_textbox_move_selectMap;//(u32 *)0x0600F800;
    for (i = 0; i < 2048; i++) {
    // only merge if there is no text on this tile
        if (!*(dst + i))
            *(dst + i) = *(src + i);
    }
}



void vblank_cb_no_merge()
{
	gpu_sprites_upload();
	copy_queue_process();
	gpu_pal_upload();
}


void c2_battle()
{
    obj_sync_superstate();
    AnimateSprites();
    process_palfade();
    RunTasks();
    // merge textbox and text tile maps
    RunTextPrinters();
}

void c2_switch_menu()
{
    obj_sync_superstate();
    AnimateSprites();
    process_palfade();
    RunTasks();
    tilemaps_sync();
    // merge textbox and text tile maps
    RunTextPrinters();
}


void handlers_clear()
{
    SetVBlankCallback(0);
    SetHBlankCallback(0);
    SetMainCallback(0);
    SetMainCallback2(0);
}


void reset_pal_settings()
{
    pal_fade_control_and_dead_struct_reset();
    palette_bg_faded_fill_black();
    gpu_pal_allocator_reset();
    *gpu_pal_tag_search_lower_boundary = 0;
}


void reset_bg_settings()
{
    overworld_free_bgmaps();
    gpu_tile_bg_drop_all_sets(0);
    bgid_mod_x_offset(0, 0, 0);
    bgid_mod_y_offset(0, 0, 0);
    bgid_mod_x_offset(1, 0, 0);
    bgid_mod_y_offset(1, 0, 0);
    bgid_mod_x_offset(2, 0, 0);
    bgid_mod_y_offset(2, 0, 0);
    bgid_mod_x_offset(3, 0, 0);
    bgid_mod_y_offset(3, 0, 0);
    HideBg(0);
    HideBg(1);
    HideBg(2);
    HideBg(3);
}

void reset_boxes()
{
    remo_reset_acknowledgement_flags();
    rboxes_free();
}


void setup()
{
    // callbacks
    handlers_clear();
    // BGs
    reset_bg_settings();
    // pals
    reset_pal_settings();
    // objs
    ResetSpriteData();
    gpu_tile_obj_tags_reset();
    // VRAM clear
    u32 set = 0;
    CpuFastSet((void*)&set, (void*)ADDR_VRAM, CPUModeFS(0x10000, CPUFSSET));
    // tasks
    malloc_init((void*)0x2000000, 0x1C000);
    ResetTasks();
    // textboxes
}
