#include <pokeagb/pokeagb.h>
#include "../battle_data/pkmn_bank.h"
#include "../battle_data/pkmn_bank_stats.h"
#include "../battle_data/battle_state.h"
#include "../moves/moves.h"
#include "../battle_text/battle_pick_message.h"
#include "battle_events/battle_events.h"
#include "../abilities/battle_abilities.h"

extern void battle_loop(void);
extern bool enqueue_message(u16 move, u8 bank, enum battle_string_ids id, u16 effect);
extern void dprintf(const char * str, ...);
extern void give_exp(u8 fainted, u8 reciever);
extern void set_active_movement(u8 tid);
extern void do_damage(u8 bank_index, u16 dmg);
extern void CpuFastSet(void* src, void* dst, u32 mode);

bool on_faint_callbacks(u8 bank)
{
    // ability on faint callbacks
    for (u8 i = 0; i < BANK_MAX; i++) {
        u8 ability = p_bank[i]->b_data.ability;
        if ((abilities[ability].on_faint) && (ACTIVE_BANK(i)))
            add_callback(CB_ON_FAINT_CHECK, 0, 0, i, (u32)abilities[ability].on_faint);
    }

    // back up cbs
    u8 old_index = CB_EXEC_INDEX;
    u32* old_execution_array = push_callbacks();
    bool old_execution_status = battle_master->executing;
    // callbacks for effectiveness of moves
    build_execution_order(CB_ON_FAINT_CHECK);
    battle_master->executing = true;
    while (battle_master->executing) {
        pop_callback(bank, CURRENT_MOVE(bank));
    }
    // restore callbacks
    restore_callbacks(old_execution_array);
    CB_EXEC_INDEX = old_index;
    battle_master->executing = old_execution_status;
    return true;
}

void obj_battler_fall_through(struct Sprite* spr)
{
    spr->data[0] += 8;
    if ((spr->data[0] < 40)) {
        spr->data[1]++;
        spr->pos1.y += 8;

        if (spr->data[6]) {
            // remove a tile layer from the bottom
            void* dst = (void*)((spr->final_oam.tile_num * 32) + 0x6010000);
            dst += 32 * 8 * (8 - spr->data[1]);
            u32 set = 0;
            CpuFastSet((void*)&set, (void*)dst, CPUModeFS(32 * 8 * spr->data[1], CPUFSSET));
        }
    } else {
        // free the hp bars too
        obj_free(&gSprites[spr->data[2]]);
        obj_free(&gSprites[spr->data[3]]);
        obj_free(&gSprites[spr->data[4]]);
        obj_free(&gSprites[spr->data[5]]);
        obj_free(spr);
        gMain.state++;
    }
}

void do_faint()
{
    u8 bank = CURRENT_ACTION->action_bank;
    switch (gMain.state) {
        case 0:
            // terminate the task that makes the player bob up and down
            if (!SIDE_OF(bank)) {
                task_del(task_find_id_by_functpr(set_active_movement));
                gSprites[p_bank[bank]->objid].data[6] = false;
            } else {
                gSprites[p_bank[bank]->objid].data[6] = ACTIVE_BANK(0) || ACTIVE_BANK(1);
            }
            // fall through the platform animation
            gSprites[p_bank[bank]->objid].callback = obj_battler_fall_through;
            gSprites[p_bank[bank]->objid].data[0] = 0;
            gSprites[p_bank[bank]->objid].data[1] = 0;
            for (u8 i = 0; i < 4; i++) {
                gSprites[p_bank[bank]->objid].data[2 + i] = p_bank[bank]->objid_hpbox[i];
                p_bank[bank]->objid_hpbox[i] = 0x3F;
            }
            p_bank[bank]->objid = 0x3F;

            enqueue_message(0, bank, STRING_FAINTED, 0);
            gMain.state++;
            break;
        case 1:
            // wait for callback to terminate
            break;
        case 2:
            on_faint_callbacks(bank);
            gMain.state++;
            break;
        case 3:
            prepend_action(bank, NULL, ActionHighPriority, EventInactive);
            end_action(CURRENT_ACTION);
            SetMainCallback(battle_loop);
            gMain.state = 0;
            break;
    };
}


void event_faint(struct action* current_action)
{
    gMain.state = 0;
    SetMainCallback(do_faint);
    return;
}


void event_fainted(struct action* current_action)
{
    for (u8 i = 0; i < BANK_MAX; i++) {
        if (!ACTIVE_BANK(i)) continue;
        if (B_IS_FAINTED(i)) {
            // faint this bank
            B_FAINTED(i) = true;
            do_damage(i, B_CURRENT_HP(i));
            prepend_action(i, NULL, ActionFaint, EventFaint);
        }
    }
    end_action(CURRENT_ACTION);
}

void event_set_inactive(struct action* current_action)
{
    ACTIVE_BANK(current_action->action_bank) = false;
    end_action(CURRENT_ACTION);
}
