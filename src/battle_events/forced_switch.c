#include <pokeagb/pokeagb.h>
#include "../battle_data/pkmn_bank.h"
#include "../battle_data/pkmn_bank_stats.h"
#include "../battle_data/battle_state.h"
#include "../moves/moves.h"
#include "../battle_text/battle_pick_message.h"
#include "battle_events/battle_events.h"

extern void jump_switch_menu(enum switch_reason reason);
extern void set_active_movement(u8 tid);


void fly_out_player_mon(struct Sprite* spr)
{
    spr->pos1.x -= 8;
    if (spr->pos1.x < -64) {
        spr->callback = NULL;
        // Free HP box and bars
        u8 bank = spr->data[0];
        for (u8 j = 0; j < 4; j++) {
            if (p_bank[bank]->objid_hpbox[j] < 0x3F)
                obj_free(&gSprites[p_bank[bank]->objid_hpbox[j]]);
            p_bank[bank]->objid_hpbox[j] = 0x3F;
        }

        // Free player pkmn
        p_bank[bank]->objid = 0x3F;
        obj_free(spr);
        jump_switch_menu(ForcedSwitch);
    }
}

void simple_delete_player_mon(struct Sprite* spr)
{
    spr->callback = NULL;
    // Free HP box and bars
    u8 bank = spr->data[0];
    for (u8 j = 0; j < 4; j++) {
        if (p_bank[bank]->objid_hpbox[j] < 0x3F)
            obj_free(&gSprites[p_bank[bank]->objid_hpbox[j]]);
        p_bank[bank]->objid_hpbox[j] = 0x3F;
    }

    // Free player pkmn
    p_bank[bank]->objid = 0x3F;
    obj_free(spr);
    jump_switch_menu(ForcedSwitch);
}

void forced_switch(u8 bank, u8 switch_method)
{
    battle_master->switch_main.reason = ForcedSwitch;
    task_del(task_find_id_by_functpr(set_active_movement));
    if (switch_method == 1) {
        gSprites[p_bank[bank]->objid].callback = simple_delete_player_mon;
        gSprites[p_bank[bank]->objid].data[0] = bank;
    } else {
        gSprites[p_bank[bank]->objid].callback = fly_out_player_mon;
        gSprites[p_bank[bank]->objid].data[0] = bank;
    }
    prepend_action(bank, bank, ActionHighPriority, EventSwitch);
    end_action(CURRENT_ACTION);
    SetMainCallback(NULL);
}

void event_forced_switch(struct action* current_action)
{
    forced_switch(CURRENT_ACTION->action_bank, CURRENT_ACTION->priv[0]);
}
