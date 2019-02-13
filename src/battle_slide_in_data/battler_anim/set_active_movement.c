#include <pokeagb/pokeagb.h>
#include "battle_data/pkmn_bank.h"
#include "battle_data/battle_state.h"

void set_active_movement(u8 t_id) {
    struct Task* t = &tasks[t_id];
   
    // calculate bank and delta
    u8 bank = t->priv[0];
    s8 amount = (t->priv[1]) ? -1 : 1;
    
    // apply delta
    if ((t->priv[2] == 10)) {
        gSprites[p_bank[bank]->objid].pos1.y += amount;
        t->priv[2]++;
    } else if (t->priv[2] == 20) {
        gSprites[p_bank[bank]->objid_hpbox[0]].pos1.y += amount;
        gSprites[p_bank[bank]->objid_hpbox[1]].pos1.y += amount;
        gSprites[p_bank[bank]->objid_hpbox[2]].pos1.y += amount;
        gSprites[p_bank[bank]->objid_hpbox[3]].pos1.y += amount;
        t->priv[2] = 0;
        // adjust direction of delta next time
        t->priv[1] = !(t->priv[1]);
    } else {
        t->priv[2]++;
        return;
    }
    
}

