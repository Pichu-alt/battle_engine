#include <pokeagb/pokeagb.h>
#include "../battle_slide_in_data/battle_obj_sliding.h"
#include "../battle_data/pkmn_bank.h"
#include "../battle_data/pkmn_bank_stats.h"
#include "../battle_data/battle_state.h"
#include "../battle_text/battle_textbox_gfx.h"
#include "../hpboxes/hpbox_positional_data.h"

extern void battle_loop(void);
extern void c2_battle(void);
extern void vblank_cb_merge_tbox(void);
extern const struct BgConfig bg_config_data[4];
extern void pick_and_load_battle_bgs_no_entry(const void* txtbox);
extern void CpuFastSet(void*, void*, u32);
extern void create_sprites_battle_mons_wild(void);
extern void option_selection(u8 bank);
extern void handlers_clear(void);
extern void reset_bg_settings(void);
extern void validate_player_selected_move(void);
extern void set_active_movement(u8 tid);

void return_to_battle()
{
    switch(gMain.state) {
        case 0:
            if (!gPaletteFade.active) {
                u32 set = 0;
                CpuFastSet((void*)&set, (void*)ADDR_VRAM, CPUModeFS(0x10000, CPUFSSET));
                SetVBlankCallback((MainCallback)vblank_cb_merge_tbox);
                SetMainCallback2((MainCallback)c2_battle);
                bg_vram_setup(0, (struct BgConfig *)&bg_config_data, 4);
                rbox_init_from_templates((struct TextboxTemplate*)0x8248330);
                if (battle_master->switch_main.reason == ViewPokemon)
                    pick_and_load_battle_bgs_no_entry(battle_textbox_action_selectMap);
                else
                    pick_and_load_battle_bgs_no_entry(battle_textboxMap);
                gMain.state++;
            }
            break;
        case 1:
            BeginNormalPaletteFade(0xFFFFFFFF, 0, 16, 0, 0x0000);
            // show bgs for background and entry image
            ShowBg(3);
            ShowBg(0);
            // show oams
            for (u8 i = 0; i < BANK_MAX; i++) {
                // show pokemon OAMs
                if (p_bank[i]->objid < 0x3F) {
                    OBJID_SHOW(p_bank[i]->objid);
                    gSprites[p_bank[i]->objid].final_oam.h_flip = false;
                    gSprites[p_bank[i]->objid].final_oam.v_flip = false;
                }
                // show HP box OAMs
                for (u8 j = 0; j < 4; j++) {
                    if (p_bank[i]->objid_hpbox[j] < 0x3F)
                        OBJID_SHOW(p_bank[i]->objid_hpbox[j]);
                }
            }
            gMain.state++;
            break;
        case 2:
            if (gPaletteFade.active) return;
            gMain.state++;
        case 3:
            switch (battle_master->switch_main.reason) {
                case ViewPokemon:
                    tasks[task_add(set_active_movement, 1)].priv[0] = battle_master->option_selecting_bank;
                    option_selection(0);
                    return;
                case NormalSwitch:
                    tasks[task_add(set_active_movement, 1)].priv[0] = battle_master->option_selecting_bank;
                    p_bank[PLAYER_SINGLES_BANK]->b_data.is_switching = true;
                    p_bank[PLAYER_SINGLES_BANK]->this_pkmn = &party_player[battle_master->switch_main.position];
                    gMain.state = 0;
                    validate_player_selected_move();
                    end_action(CURRENT_ACTION);
                    SetMainCallback(battle_loop);
                    return;
                case ForcedSwitch:
                    p_bank[PLAYER_SINGLES_BANK]->this_pkmn = &party_player[battle_master->switch_main.position];
                    gMain.state = 0;
                    p_bank[PLAYER_SINGLES_BANK]->b_data.is_switching = false;
                    SetMainCallback(battle_loop);
                    return;
                case PokemonFainted:
                    p_bank[PLAYER_SINGLES_BANK]->this_pkmn = &party_player[battle_master->switch_main.position];
                    gMain.state = 0;
                    CURRENT_ACTION->event_state++;
                    SetMainCallback(battle_loop);
                    return;
            };
            dprintf("INVALID SWITCH REASON WAS GIVEN\n");
            gMain.state++;
            break;
        case 4:
            break;
    };

}

void (*dp12_8087EA4)(void) = 0x8087EA4|1;
extern void reset_pal_settings(void);
extern u8 spawn_hpbox_opponent(u16 tag, s16 x, s16 y, u8 bank);
extern u8 spawn_hpbox_player(u16 tag, s16 x, s16 y, u8 bank);
void return_to_battle_bag()
{
    switch (gMain.state) {
        case 0:
            // display
            lcd_io_set(0x4C, 0);
            REG_WININ = WININ_BUILD(WIN_BG0 | WIN_BG1 | WIN_BG2 | WIN_BG3 | WIN_OBJ | WIN_BLD,
            WIN_BG0 | WIN_BG1 | WIN_BG2 | WIN_BG3 | WIN_OBJ | WIN_BLD);
            rboxes_free();

            // callbacks
            handlers_clear();
            SetMainCallback(return_to_battle_bag);
            SetMainCallback2((MainCallback)c2_battle);
            SetVBlankCallback((MainCallback)vblank_cb_merge_tbox);

            // BG stuff
            u32 set = 0;
            CpuFastSet((void*)&set, (void*)ADDR_VRAM, CPUModeFS(0x10000, CPUFSSET));
            reset_pal_settings();
            gpu_tile_bg_drop_all_sets(0);
            bg_vram_setup(0, (struct BgConfig *)&bg_config_data, 4);
            // BGs
            pick_and_load_battle_bgs_no_entry(battle_textbox_action_selectMap);
            HideBg(0);
            HideBg(1);
            HideBg(2);
            HideBg(3);
            // bag clear something
            u8* bag_something = (u8*)0x2023FE5;
            u8* bag_something2 = (u8*)0x2023FE6;
            *bag_something = 0;
            *bag_something2 = 0;
            dp12_8087EA4();
            gMain.state++;
            break;
        case 1:
            {
            // objs
            for (u8 i = 0; i < 0x3F; i++) {
                obj_free(&gSprites[i]);
            }
            ResetSpriteData();
            gpu_tile_obj_tags_reset();
            // need to redraw gSprites
            create_sprites_battle_mons_wild();
            gSprites[p_bank[OPPONENT_SINGLES_BANK]->objid].pos1.x = 178;
            // spawn the HP boxes
            spawn_hpbox_opponent(HPBOX_TAG_OPP_SW, HPBOX_OPP_SW_X, HPBOX_OPP_SW_Y, OPPONENT_SINGLES_BANK);
            spawn_hpbox_player(HPBOX_TAG_PLAYER_SINGLE, HPBOX_PLAYER_SINGLE_X, HPBOX_PLAYER_SINGLE_Y, PLAYER_SINGLES_BANK);
            ShowBg(0);
            ShowBg(3);
            BeginNormalPaletteFade(0xFFFFFFFF, 0, 16, 0, 0x0000);
            gMain.state++;
            }
            break;
        case 2:
            if (!gPaletteFade.active) {
                // continue game callbacks
                // init textbox
                rbox_init_from_templates((struct TextboxTemplate*)0x8248330);
                gMain.state++;
            }
            break;
        default:
            // set active movements
            tasks[task_add(set_active_movement, 1)].priv[0] = PLAYER_SINGLES_BANK;
            option_selection(0);
            break;
    };

}
