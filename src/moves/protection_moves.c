#include <pokeagb/pokeagb.h>
#include "../battle_data/pkmn_bank.h"
#include "../battle_data/pkmn_bank_stats.h"
#include "../battle_data/battle_state.h"

extern u16 rand_range(u16, u16);
extern bool enqueue_message(u16 move, u8 bank, enum battle_string_ids id, u16 effect);
extern bool do_damage_residual(u8 bank_index, u16 dmg, u32 ability_flags);
void set_status(u8 bank, enum Effect status, u8 inflictor);
extern void stat_boost(u8 bank, u8 stat_id, s8 amount, u8 inflicting_bank);
extern bool protection_effect_exists_side(u8 bank, u32 func);
extern bool moves_last(u8 bank);

const static u8 chances_protect[] = {100, 33, 3, 1};
const static u16 protection_moves[] = {
    MOVE_BANEFULBUNKER, MOVE_DETECT, MOVE_ENDURE,
    MOVE_KINGSSHIELD, MOVE_PROTECT, MOVE_QUICKGUARD,
    MOVE_SPIKYSHIELD, MOVE_WIDEGUARD};

bool is_protection_move(u16 move)
{
    for (u8 i = 0; i < (sizeof(protection_moves) / sizeof(u16)); i++) {
        if (protection_moves[i] == move)
            return true;
    }
    return false;
}

/* Protect and Detect */
// on tryhit try to see if protect will fail
enum TryHitMoveStatus protection_on_tryhit(u8 user, u8 src, u16 move, struct anonymous_callback* acb)
{
    if (user != src) return TRYHIT_USE_MOVE_NORMAL;
    u8 chance_land = 0;
    if (!is_protection_move(LAST_MOVE(user)))
        PROTECTION_TURNS(user) = 0;
    if (PROTECTION_TURNS(user) < 4) {
        chance_land = chances_protect[PROTECTION_TURNS(user)];
    }
    if (rand_range(0, 100) > chance_land) {
        PROTECTION_TURNS(user) = 0;
        return TRYHIT_CANT_USE_MOVE; // move failed to land
    } else {
        // move landed
        PROTECTION_TURNS(user)++;
        return TRYHIT_USE_MOVE_NORMAL;
    }
}

u8 protect_on_tryhit_anon(u8 user, u8 source, u16 move, struct anonymous_callback* acb)
{
    if (TARGET_OF(user) != source) return true;
    if (IS_PROTECTABLE(move)) {
        enqueue_message(0, TARGET_OF(user), STRING_PROTECTED_SELF, 0);
        return 3; // fail the move silently
    }
    return true;
}

u8 protect_on_effect(u8 user, u8 src, u16 move, struct anonymous_callback* acb)
{
    // msg: X protected itself
    // queue an anon func to read and interrupt
    if (user != src) return true;
    enqueue_message(0, src, STRING_PROTECTED_SELF, 0);
    if ((CURRENT_MOVE(user) == MOVE_PROTECT) || (CURRENT_MOVE(user) == MOVE_DETECT)) {
        add_callback(CB_ON_TRYHIT_MOVE, 4, 0, user, (u32)(protect_on_tryhit_anon));
    } else if ((CURRENT_MOVE(user) == MOVE_SPIKYSHIELD)) {
        extern u8 spiky_shield_on_tryhit_anon(u8 user, u8 source, u16 move, struct anonymous_callback* acb);
        add_callback(CB_ON_TRYHIT_MOVE, 4, 0, user, (u32)spiky_shield_on_tryhit_anon);
    } else if ((CURRENT_MOVE(user) == MOVE_BANEFULBUNKER)) {
        extern u8 baneful_bunker_on_tryhit_anon(u8 user, u8 source, u16 move, struct anonymous_callback* acb);
        add_callback(CB_ON_TRYHIT_MOVE, 4, 0, user, (u32)baneful_bunker_on_tryhit_anon);
    } else {
        extern u8 kings_shield_on_tryhit_anon(u8 user, u8 source, u16 move, struct anonymous_callback* acb);
        add_callback(CB_ON_TRYHIT_MOVE, 4, 0, user, (u32)kings_shield_on_tryhit_anon);
    }
    return true;
}

/* Spiky Shield */
// rest of callbacks are shared with protect/detect
u8 spiky_shield_on_tryhit_anon(u8 user, u8 source, u16 move, struct anonymous_callback* acb)
{
    if (TARGET_OF(user) != source) return true;
    if (IS_PROTECTABLE(move)) {
        enqueue_message(0, TARGET_OF(user), STRING_PROTECTED_SELF, 0);
        if(IS_CONTACT(move)) {
            if(do_damage_residual(user, TOTAL_HP(TARGET_OF(user)) / 8, NULL))
                enqueue_message(MOVE_SPIKYSHIELD, user, STRING_RESIDUAL_DMG, MOVE_SPIKYSHIELD);
        }
        return 3; // fail the move silently
    }
    return true;
}

/* Baneful Bunker */
// rest of callbacks are shared with protect/detect
u8 baneful_bunker_on_tryhit_anon(u8 user, u8 source, u16 move, struct anonymous_callback* acb)
{
    if (TARGET_OF(user) != source) return true;
    if (IS_PROTECTABLE(move)) {
        enqueue_message(0, TARGET_OF(user), STRING_PROTECTED_SELF, 0);
        if(IS_CONTACT(move))
            set_status(user, AILMENT_POISON, source);
        return 3; // fail the move silently
    }
    return true;
}

/* King's shield*/
u8 kings_shield_on_tryhit_anon(u8 user, u8 source, u16 move, struct anonymous_callback* acb)
{
    if (TARGET_OF(user) != source) return true;
    if (IS_PROTECTABLE(move)) {
        enqueue_message(0, TARGET_OF(user), STRING_PROTECTED_SELF, 0);
        if(IS_CONTACT(move))
            stat_boost(user, 0, -2, TARGET_OF(user));
        return 3; // fail the move silently
    }
    return true;
}


/* Mat block */
enum TryHitMoveStatus mat_block_on_tryhit(u8 user, u8 source, u16 move, struct anonymous_callback* acb)
{
    if (user != source) return TRYHIT_USE_MOVE_NORMAL;
    u8 status = protection_on_tryhit(user, source, move, acb);
    if (status && p_bank[user]->b_data.first_turn)
        return TRYHIT_USE_MOVE_NORMAL;
    return TRYHIT_CANT_USE_MOVE;
}

u8 mat_block_on_tryhit_anon(u8 user, u8 source, u16 move, struct anonymous_callback* acb)
{
    if (TARGET_OF(user) != source) return true;
    if (IS_PROTECTABLE(move)) {
        enqueue_message(0, TARGET_OF(user), STRING_KICKED_UP_MAT, 0);
        return 3; // fail the move silently
    }
    return true;
}

u8 mat_block_on_effect(u8 user, u8 src, u16 move, struct anonymous_callback* acb)
{
    // msg: X protected itself
    // queue an anon func to read and interrupt
    if (user != src) return true;
    // fail if user is last to move
    if (moves_last(src)) return false;
    if (protection_effect_exists_side(src, (u32)mat_block_on_tryhit_anon)) return false;
    enqueue_message(MOVE_MATBLOCK, src, STRING_PROTECTED_TEAM, 0);
    add_callback(CB_ON_TRYHIT_MOVE, 3, 0, user, (u32)mat_block_on_tryhit_anon);
    return true;
}


/* Wide guard */
u8 wide_guard_on_tryhit_anon(u8 user, u8 source, u16 move, struct anonymous_callback* acb)
{
    if (SIDE_OF(TARGET_OF(user)) != SIDE_OF(source)) return true;
    if (IS_PROTECTABLE(move) && M_HITS_FOE_SIDE(move)) {
        enqueue_message(MOVE_WIDEGUARD, TARGET_OF(user), STRING_PROTECTED_MON, 0);
        return 3; // fail the move silently
    }
    return true;
}

u8 wide_guard_on_effect(u8 user, u8 src, u16 move, struct anonymous_callback* acb)
{
    // msg: X protected itself
    // queue an anon func to read and interrupt
    if (user != src) return true;
    enqueue_message(MOVE_WIDEGUARD, src, STRING_PROTECTED_TEAM, 0);
    add_callback(CB_ON_TRYHIT_MOVE, 3, 0, user, (u32)wide_guard_on_tryhit_anon);
    return true;
}

/* Quick Guard */
u8 quick_guard_on_tryhit_anon(u8 user, u8 src, u16 move, struct anonymous_callback* acb)
{
    if (SIDE_OF(user) == SIDE_OF(src)) return true;
    if ((B_MOVE_PRIORITY(user) > 0) && (SIDE_OF(TARGET_OF(user)) == SIDE_OF(src))) {
        enqueue_message(MOVE_QUICKGUARD, TARGET_OF(user), STRING_PROTECTED_MON, 0);
        return 3; // fail the move silently
    }
    return true;
}

u8 quick_guard_on_effect(u8 user, u8 src, u16 move, struct anonymous_callback* acb)
{
    // msg: X protected itself
    // queue an anon func to read and interrupt
    if (user != src) return true;
    enqueue_message(MOVE_QUICKGUARD, src, STRING_PROTECTED_TEAM, 0);
    add_callback(CB_ON_TRYHIT_MOVE, 3, 0, user, (u32)quick_guard_on_tryhit_anon);
    return true;
}

/* Crafty shield */
u8 crafty_shield_on_tryhit_anon(u8 user, u8 source, u16 move, struct anonymous_callback* acb)
{
    if (SIDE_OF(TARGET_OF(user)) != SIDE_OF(source)) return true;
    if (B_MOVE_IS_STATUS(user)) {
        enqueue_message(MOVE_CRAFTYSHIELD, TARGET_OF(user), STRING_PROTECTED_MON, 0);
        return 3; // fail the move silently
    }
    return true;
}

enum TryHitMoveStatus crafty_shield_on_tryhit(u8 user, u8 source, u16 move, struct anonymous_callback* acb)
{
    if (user != source) return TRYHIT_USE_MOVE_NORMAL;
    if (protection_effect_exists_side(source, (u32)crafty_shield_on_tryhit_anon)) return TRYHIT_CANT_USE_MOVE;
    return (!moves_last(source));
}

u8 crafty_shield_on_effect(u8 user, u8 src, u16 move, struct anonymous_callback* acb)
{
    // msg: X protected itself
    // queue an anon func to read and interrupt
    if (user != src) return true;
    enqueue_message(MOVE_CRAFTYSHIELD, src, STRING_PROTECTED_TEAM, 0);
    add_callback(CB_ON_TRYHIT_MOVE, 3, 0, user, (u32)crafty_shield_on_tryhit_anon);
    return true;
}

/* Endure */
// tryhit with protection moves is shared
void endure_on_damage(u8 user, u8 source, u16 move, struct anonymous_callback* acb)
{
    if (TARGET_OF(user) != source) return;
    u16 dmg = B_MOVE_DMG(user);
    if (dmg > B_CURRENT_HP(source)) {
        B_MOVE_DMG(user) = B_CURRENT_HP(source) - 1;
        enqueue_message(0, source, STRING_ENDURED, 0);
    }
}

u8 endure_on_effect(u8 user, u8 src, u16 move, struct anonymous_callback* acb)
{
    // msg: braced itself
    if (user != src) return true;
    enqueue_message(0, src, STRING_BRACED_ITSELF, 0);
    add_callback(CB_ON_DAMAGE_MOVE, -10, 0, user, (u32)endure_on_damage);
    return true;
}


/* Protection breaking */
const static u32 protects_to_break[] = {
    (u32)protect_on_tryhit_anon, (u32)spiky_shield_on_tryhit_anon,
    (u32)baneful_bunker_on_tryhit_anon, (u32)kings_shield_on_tryhit_anon,
    (u32)mat_block_on_tryhit_anon, (u32)wide_guard_on_tryhit_anon
};

bool break_protection(u8 bank)
{
    bool broke_protection = false;
    for (u8 i = 0; i < (sizeof(protects_to_break) / sizeof(u32)); i++) {
        u8 protect_index = id_by_func(protects_to_break[i]);
        if (protect_index != 255) {
            // valid ID. Check is it's source is bank, then mark inactive
            if (CB_MASTER[protect_index].in_use && (CB_MASTER[protect_index].source_bank == bank)) {
                CB_MASTER[protect_index].in_use = false;
                broke_protection = true;
            }
        }
    }
    return broke_protection;
}

bool protection_effect_exists_side(u8 bank, u32 func)
{
    u8 i = id_by_func(func);
    if (i == 255) return false;
    if (SIDE_OF(CB_MASTER[i].source_bank) == SIDE_OF(bank)) return true;
    return false;
}
