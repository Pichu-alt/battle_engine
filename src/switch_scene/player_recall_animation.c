#include <pokeagb/pokeagb.h>
#include "../battle_data/pkmn_bank.h"
#include "../battle_data/pkmn_bank_stats.h"
#include "../battle_data/battle_state.h"
#include "../moves/moves.h"
#include "../battle_text/battle_pick_message.h"
#include "battle_events/battle_events.h"
#include "../battle_slide_in_data/battle_obj_sliding.h"

extern void update_pbank(u8 bank, struct update_flags* flags);
extern void buffer_write_pkmn_nick(pchar* buff, u8 bank);
extern void make_spinning_pokeball(s16 x, s16 y, u8 bank);
extern void set_active_movement(u8 tid);
extern void player_hpbar_slidin_slow(u8 t_id);
extern u8 spawn_hpbox_player(u16 tag, s16 x, s16 y, u8 bank);
extern void battle_loop(void);
extern void run_after_switch(u8 bank);
extern void jump_switch_menu(enum switch_reason reason);

static const struct RotscaleFrame shrink[] = {
    {-10, -10, 0, 10, 0},
    {-20, -20, 0, 5, 0},
    {0x7FFF, 0, 0, 0, 0},
};

static const struct RotscaleFrame* shrink_ptr[] = {shrink};


void reset_hpbars_and_sprite_player(struct Sprite* spr)
{
    // Free HP box and bars
    u8 bank = CURRENT_ACTION->action_bank;
    for (u8 j = 0; j < 4; j++) {
        if (p_bank[bank]->objid_hpbox[j] < 0x3F) {
            battle_master->switch_main.type_objid[j] = p_bank[bank]->objid_hpbox[j];
            OBJID_HIDE(p_bank[bank]->objid_hpbox[j]);
        } else {
            battle_master->switch_main.type_objid[j] = 0x3F;
        }
        p_bank[bank]->objid_hpbox[j] = 0x3F;
    }

    // Free player pkmn
    p_bank[bank]->objid = 0x3F;
    obj_free(spr);
}


void pkmn_recall_cb(struct Sprite* spr)
{
    if (spr->data[0] < 25) {
        spr->pos1.y += 2;
    } else {
        reset_hpbars_and_sprite_player(spr);
    }
    spr->data[0]++;
}


void pkmn_player_normal_switch()
{
    switch (gMain.state) {
        case 0:
            {
                u8 bank = CURRENT_ACTION->action_bank;
                u8 objid = p_bank[bank]->objid;
                gSprites[objid].rotscale_table = shrink_ptr;
                gSprites[objid].data[0] = 0;
                gSprites[objid].callback = pkmn_recall_cb;
                OBJID_SHOW_AFFINE(objid);
                obj_rotscale_play(&gSprites[objid], 0);
                // fade only this OAM's palette to pink
                u8 pal_slot = gSprites[objid].final_oam.palette_num;
                u32 pal_fade = ((1 << (pal_slot + 16)));
                BeginNormalPaletteFade(pal_fade , 2, 0x10, 0x0, 0x7ADF);
                gMain.state++;
                break;
            }
        case 1:
            {
                task_del(task_find_id_by_functpr(set_active_movement));
                gMain.state++;
                break;
            }
        case 2:
            {
                if (p_bank[CURRENT_ACTION->action_bank]->objid < 0x3F) return;
                pchar text[] = _("Go! {STR_VAR_2}!");
                memcpy(fcode_buffer3, p_bank[CURRENT_ACTION->action_bank]->this_pkmn->box.nick, sizeof(party_player[0].box.nick));
                fcode_buffer3[sizeof(party_player[0].box.nick)] = 0xFF;
                fdecoder(string_buffer, text);
                remo_reset_acknowledgement_flags();
                battle_show_message((u8*)string_buffer, 0x18);
                gMain.state++;
                break;
            }
       case 3:
            if (!dialogid_was_acknowledged(0x18 & 0x3F)) {
                u8 bank = CURRENT_ACTION->action_bank;
                make_spinning_pokeball(53, 64, bank);
                bs_anim_status = 1;
                for (u8 i = 0; i < 4; i++) {
                    u8 objid = battle_master->switch_main.type_objid[i];
                    if (objid < 0x3F)
                        obj_free(&gSprites[objid]);
                    battle_master->switch_main.type_objid[i] = 0x3F;
                }

                // spawn new HP bar
                spawn_hpbox_player(HPBOX_TAG_PLAYER_SINGLE, HPBOX_PLAYER_SINGLE_X, HPBOX_PLAYER_SINGLE_Y, bank);
                gSprites[p_bank[bank]->objid_hpbox[0]].pos1.x += 128;
                gSprites[p_bank[bank]->objid_hpbox[1]].pos1.x += 128;
                gSprites[p_bank[bank]->objid_hpbox[2]].pos1.x += 128;
                gSprites[p_bank[bank]->objid_hpbox[3]].pos1.x += 128;
                gMain.state++;
            }
            break;
        case 4:
            if ((gPaletteFade.active) || (bs_anim_status)) return;
            task_add(player_hpbar_slidin_slow, 1);
            bs_anim_status = 1;
            gMain.state++;
            break;
        case 5:
            if (bs_anim_status) return;
            struct update_flags* flags = (struct update_flags*)malloc_and_clear(sizeof(struct update_flags));
            flags->pass_status = false;
            flags->pass_stats = false;
            flags->pass_atk_history = false;
            flags->pass_disables = false;
            update_pbank(CURRENT_ACTION->action_bank, flags);
            free(flags);
            gMain.state++;
            break;
        case 6:
            if (!gPaletteFade.active) {
                run_after_switch(CURRENT_ACTION->action_bank);
                p_bank[CURRENT_ACTION->action_bank]->b_data.is_switching = false;
                u8 t_id = task_add(set_active_movement, 1);
                tasks[t_id].priv[0] = CURRENT_ACTION->action_bank;
                end_action(CURRENT_ACTION);
                gMain.state = 0;
                SetMainCallback(battle_loop);
            }
            break;
    };
}


void pkmn_recall_animation()
{
    switch (battle_master->switch_main.reason) {
        case ViewPokemon:
            dprintf("Reason ViewPokemon given for switch into battle. Executing normal switch.");
        case NormalSwitch:
            gMain.state = 0;
            SetMainCallback(pkmn_player_normal_switch);
            return;
        case ForcedSwitch:
            gMain.state = 2;
            SetMainCallback(pkmn_player_normal_switch);
            break;
        case PokemonFainted:
            gMain.state = 2;
            SetMainCallback(pkmn_player_normal_switch);
            return;
    };
}
