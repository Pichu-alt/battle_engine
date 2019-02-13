#include <pokeagb/pokeagb.h>
#include "battle_data/pkmn_bank.h"
#include "battle_data/pkmn_bank_stats.h"
#include "battle_data/battle_state.h"
#include "battle_text/battle_textbox_gfx.h"

extern void validate_player_selected_move(void);
extern void vblank_cb_no_merge(void);
extern void vblank_cb_merge_move_select(void);
extern void vblank_cb_merge_tbox(void);
extern void load_icons_moves(u8 bank);
extern void update_cursor_move_select(u8 task_id);
extern void show_move_data(void);
extern void init_selection_cursor(u8 mode, u8 bank);
extern void dprintf(const char * str, ...);
extern void option_selection2(void);
extern void switch_scene_main(void);
extern void free_unused_objs(void);
extern void free_battler_oams(void);
extern void event_peek_message(struct action* current_action);
extern void CpuFastSet(void* src, void* dst, u32 mode);
extern void sync_battler_struct(u8 bank);
extern void set_active_movement(u8 task_id);
extern void return_to_battle_bag(void);

void (*sub_8107ECC)(u8, u8, MainCallback) = 0x8107DB5;


/* Fight menu and move menu selection. Preperation to go into battle loop*/

#define SELECTING_BANK battle_master->option_selecting_bank

enum BattleMenuSelectionOptions {
    BaseMenuInitialize = 0,
    BaseMenuInputInterpret,
    FightOptionSelected_FirstTime,
    FightOptionSelected_FastLoad,
    SwitchOptionSelected,
    BagOptionSelected,
    RunOptionSelected,
    MenuWaitState,
    MoveSelectedExit,
};

/* Fight menu and move menu selection. Preperation to go into battle loop*/
void option_selection(u8 bank)
{
    battle_master->option_selecting_bank = bank;
    // check bank is skipping selection via move
    if ((HAS_VOLATILE(bank, VOLATILE_CHARGING)) ||
        (p_bank[bank]->b_data.skip_move_select) ||
        (HAS_VOLATILE(bank, VOLATILE_RECHARGING))) {
        // jump straight to move validation
        SetMainCallback(validate_player_selected_move);
        return;
    }
    SetMainCallback(option_selection2);
    gMain.state = BaseMenuInitialize;
}

void jump_switch_menu(enum switch_reason reason)
{
    BeginNormalPaletteFade(0xFFFFFFFF, 0, 0, 16, 0x0000);
    task_del(task_find_id_by_functpr(set_active_movement));
    free_unused_objs();
    battle_master->switch_main.position = 0;
    battle_master->fight_menu_content_spawned  = 0;
    battle_master->switch_main.reason = reason;
    gMain.state = 0;
    sync_battler_struct(battle_master->option_selecting_bank);
    SetMainCallback(switch_scene_main);
}

void bag_prep()
{
    if (!gPaletteFade.active) {
        SetMainCallback(NULL);
        sub_8107ECC(5, 3, return_to_battle_bag);
        gpu_tile_bg_drop_all_sets(0);
        rboxes_free();
        free_battler_oams();
    }
}

void option_selection2()
{
    // the new peek message
    if (ACTION_HEAD != NULL) {
        CURRENT_ACTION = ACTION_HEAD;
        event_peek_message(ACTION_HEAD);
        return;
    }
    switch (gMain.state) {
        case BaseMenuInitialize:
        {
            // initialize fight menu selection
            SetVBlankCallback(vblank_cb_no_merge);
            void* map_base = (void *)0x600F800;
            //memcpy(map_base, battle_textbox_action_selectMap, sizeof(battle_textbox_action_selectMap));
            CpuFastSet((void*)&battle_textbox_action_selectMap, (void*)map_base, CPUModeFS(0x800, CPUFSCPY));
            init_selection_cursor(1, 0);

            // next state
            bs_anim_status = 1;
            gMain.state = BaseMenuInputInterpret;
            break;
        }
        case BaseMenuInputInterpret:
            // wait for input selection from fight menu
            if (bs_anim_status)
                return;
            // interpret selection
            switch (battle_master->selected_option) {
                case OPTION_FIGHT:
                    if (battle_master->fight_menu_content_spawned) {
                        gMain.state = FightOptionSelected_FastLoad;
                    } else {
                        gMain.state = FightOptionSelected_FirstTime;
                    }
                    break;
                case OPTION_POKEMON:
                    gMain.state = SwitchOptionSelected;
                    break;
                case OPTION_BAG:
                    gMain.state = BagOptionSelected;
                    break;
                case OPTION_RUN:
                    gMain.state = RunOptionSelected;
                    break;

            };
            break;
        case FightOptionSelected_FirstTime:
            /* FIGHT selected from fight menu */

            // update tilemap
            SetVBlankCallback(vblank_cb_merge_move_select);
            void* map_base = (void *)0x600F800;
            //memcpy(map_base, battle_textbox_move_selectMap, sizeof(battle_textbox_move_selectMap));
            CpuFastSet((void*)&battle_textbox_move_selectMap, (void*)map_base, CPUModeFS(0x800, CPUFSCPY));
            // init cursor
            init_selection_cursor(0, 0);

            // init move types
            load_icons_moves(SELECTING_BANK);
            // set into pause state
            bs_anim_status = 1;
            gMain.state = MenuWaitState;
            break;
        case FightOptionSelected_FastLoad:
            {
                SetVBlankCallback(vblank_cb_merge_move_select);
                void* map_base = (void *)0x600F800;
                //memcpy(map_base, battle_textbox_move_selectMap, sizeof(battle_textbox_action_selectMap));
                CpuFastSet((void*)&battle_textbox_move_selectMap, (void*)map_base, CPUModeFS(0x800, CPUFSCPY));
                show_move_data();
                tasks[task_add(update_cursor_move_select, 1)].priv[0] = 0;
                bs_anim_status = 1;
                gMain.state = MenuWaitState;
                break;
            }
        case SwitchOptionSelected:
            // POKEMON selection from fight menu
            jump_switch_menu(ViewPokemon);
            break;
        case BagOptionSelected:
            // BAG selected from fight menu
            BeginNormalPaletteFade(0xFFFFFFFF, 0, 0, 16, 0x0000);
            task_del(task_find_id_by_functpr(set_active_movement));
            free_unused_objs();
            battle_master->fight_menu_content_spawned  = 0;
            gMain.state = 0;
            SetMainCallback(bag_prep);
            break;
        case RunOptionSelected:
        {
            // RUN selected from fight menu
            p_bank[SELECTING_BANK]->b_data.is_running = true;
            gMain.state = MoveSelectedExit;
            break;
        }
        case MenuWaitState:
            break;
        case MoveSelectedExit:
        {
            free_unused_objs();
            SetMainCallback(validate_player_selected_move);
            SetVBlankCallback((MainCallback)vblank_cb_merge_tbox);
            battle_master->fight_menu_content_spawned  = 0;
            gMain.state = 0;
            break;
        }
        case 9:
            // pick move validator function based on battle type
            return;
    };
}
