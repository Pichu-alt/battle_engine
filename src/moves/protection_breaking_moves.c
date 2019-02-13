#include <pokeagb/pokeagb.h>
#include "../battle_data/pkmn_bank.h"
#include "../battle_data/pkmn_bank_stats.h"
#include "../battle_data/battle_state.h"

extern bool break_protection(u8 bank);
extern bool enqueue_message(u16 move, u8 bank, enum battle_string_ids id, u16 effect);

u8 feint_on_before_move(u8 user, u8 src, u16 move, struct anonymous_callback* acb)
{
    if (user != src) return true;
    if (break_protection(TARGET_OF(src))) {
        enqueue_message(MOVE_FEINT, src, STRING_BROKE_PROTECT, 0);
    }
    return true;
}

u8 hyperspace_hole_before_move(u8 user, u8 src, u16 move, struct anonymous_callback* acb)
{
    if (user != src) return true;
    if (break_protection(TARGET_OF(src))) {
        enqueue_message(MOVE_HYPERSPACEHOLE, src, STRING_BROKE_PROTECT, 0);
    }
    return true;
}
