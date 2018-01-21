#ifndef BATTLE_ABILITIES_TABLE_H_
#define BATTLE_ABILITIES_TABLE_H_

#include <pokeagb/pokeagb.h>
#include "battle_data/pkmn_bank.h"
#include "battle_data/battle_state.h"
#include "moves/moves.h"
#include "ability_override.h"

typedef void (*AbilityBeforeTurn)(u8 user, u8 source, u16 move, struct anonymous_callback* acb);
typedef void (*AbilityOnStart)(u8 user, u8 source, u16 move, struct anonymous_callback* acb);
typedef u8 (*AbilityBeforeSwitchOutCallback)(u8 user, u8 source, u16 move, struct anonymous_callback* acb);
typedef u8 (*AbilityBeforeMoveCallback)(u8 user, u8 source, u16 move, struct anonymous_callback* acb);
typedef u8 (*AbilityOnFailCallback)(u8 user, u8 source, u16 move, struct anonymous_callback* acb);
typedef u8 (*AbilityOnModifyMoveCallback)(u8 user, u8 source, u16 move, struct anonymous_callback* acb);
typedef u8 (*AbilityOnTryhitCallback)(u8 user, u8 source, u16 move, struct anonymous_callback* acb);
typedef void (*AbilityOnBasePowerCallback)(u8 user, u8 source, u16 move, struct anonymous_callback* acb);
typedef void (*AbilityOnDamageCallback)(u8 user, u8 source, u16 move, struct anonymous_callback* acb);
typedef u8 (*AbilityOnEffectCallback)(u8 user, u8 source, u16 move, struct anonymous_callback* acb);
typedef void (*AbilityOnAfterMoveCallback)(u8 user, u8 source, u16 move, struct anonymous_callback* acb);
typedef u8 (*AbilityOnStatusCallback)(u8 user, u8 source, u16 ailment, struct anonymous_callback* acb);
typedef bool (*AbilityOnStatBoostModCallback)(u8 user, u8 source, u16 stat_id, struct anonymous_callback* acb);
typedef u16 (*AbilityOnStatCallback)(u8 user, u8 source, u16 move, struct anonymous_callback* acb);
typedef void (*AbilityAfterStatBoostModCallback)(u8 user, u8 source, u16 stat_id, struct anonymous_callback* acb);
typedef u8 (*AbilityOnResidualCallback)(u8 user, u8 source, u16 stat_id, struct anonymous_callback* acb);
typedef void (*AbilityOnDrainCallback)(u8 user, u8 source, u16 stat_id, struct anonymous_callback* acb);
typedef u16 (*AbilityOnEffectiveness)(u8 target_type, u8 src, u16 move_type, struct anonymous_callback* acb); // acb->data == ((attacker << 16) | move_effectiveness);
typedef void(*AbilityOnFaintCallback)(u8 user, u8 source, u16 stat_id, struct anonymous_callback* acb);

struct ability_data {
    AbilityBeforeTurn before_turn;
    AbilityOnStart on_start;
    AbilityBeforeSwitchOutCallback before_switch;
    AbilityBeforeMoveCallback before_move;
    AbilityOnFailCallback on_fail;
    AbilityOnModifyMoveCallback on_modify_move;
    AbilityOnTryhitCallback on_tryhit;
    AbilityOnEffectiveness on_effectiveness;
    AbilityOnBasePowerCallback on_base_power;
    AbilityOnDamageCallback on_damage;
    AbilityOnEffectCallback on_effect;
    AbilityOnDrainCallback on_drain;
    AbilityOnAfterMoveCallback on_after_move;
    AbilityOnStatusCallback on_status;
    AbilityOnStatCallback on_stat; // when a base stat is read. acb->data_ptr = stat total. move = stat_id
    AbilityOnStatBoostModCallback on_stat_boost_mod;
    AbilityAfterStatBoostModCallback after_stat_boost_mod;
    AbilityOnResidualCallback on_residual;
    AbilityOnFaintCallback on_faint;
};

extern struct ability_data abilities[];
extern const pchar ability_names[][17];

/* Callback externs */
extern u8 ability_none_on_effect(u8 user, u8 source, u16 move, struct anonymous_callback* acb);
extern void stench_on_damage(u8 user, u8 source, u16 move, struct anonymous_callback* acb);
extern u8 limber_on_status(u8 user, u8 source, u16 ailment , struct anonymous_callback* acb);
extern u8 insomnia_on_status(u8 user, u8 source, u16 ailment , struct anonymous_callback* acb);
extern u8 immunity_on_status(u8 user, u8 source, u16 ailment , struct anonymous_callback* acb);
extern u8 waterveil_on_status(u8 user, u8 source, u16 ailment , struct anonymous_callback* acb);
extern u8 magmaarmor_on_status(u8 user, u8 source, u16 ailment , struct anonymous_callback* acb);
extern u8 vitalspirit_on_status(u8 user, u8 source, u16 ailment , struct anonymous_callback* acb);
extern void technician_on_base_power(u8 user, u8 source, u16 move, struct anonymous_callback* acb);
extern bool contrary_on_stat_boost_mod(u8 user, u8 source, u16 stat_id, struct anonymous_callback* acb);
extern bool simple_on_stat_boost_mod(u8 user, u8 source, u16 stat_id, struct anonymous_callback* acb);
extern void defiant_after_stat_boost_mod(u8 user, u8 source, u16 stat_id, struct anonymous_callback* acb);
extern void competitive_after_stat_boost_mod(u8 user, u8 source, u16 stat_id, struct anonymous_callback* acb);
extern void steelworker_on_base_power(u8 user, u8 source, u16 move, struct anonymous_callback* acb);
extern void colorchange_on_after_move(u8 user, u8 source, u16 move, struct anonymous_callback* acb);
extern void heatproof_on_base_power(u8 user, u8 source, u16 move, struct anonymous_callback* acb);
extern void drizzle_on_start(u8 user, u8 src, u16 move, struct anonymous_callback* acb);
extern void sandstream_on_start(u8 user, u8 src, u16 move, struct anonymous_callback* acb);
extern void snowwarning_on_start(u8 user, u8 src, u16 move, struct anonymous_callback* acb);
extern void drought_on_start(u8 user, u8 src, u16 move, struct anonymous_callback* acb);
extern u8 speedboost_on_residual(u8 user, u8 src, u16 move, struct anonymous_callback* acb);
extern void ironfist_on_base_power(u8 user, u8 source, u16 move, struct anonymous_callback* acb);
extern void reckless_on_base_power(u8 user, u8 source, u16 move, struct anonymous_callback* acb);
extern void strongjaw_on_base_power(u8 user, u8 source, u16 move, struct anonymous_callback* acb);
extern void megalauncher_on_base_power(u8 user, u8 source, u16 move, struct anonymous_callback* acb);
extern void adaptability_on_base_power(u8 user, u8 source, u16 move, struct anonymous_callback* acb);
extern void tintedlens_on_damage(u8 user, u8 source, u16 move, struct anonymous_callback* acb);
extern void filter_variations_on_damage(u8 user, u8 source, u16 move, struct anonymous_callback* acb);
extern u8 flamebody_on_effect(u8 user, u8 source, u16 move, struct anonymous_callback* acb);
extern u8 poisonpoint_on_effect(u8 user, u8 source, u16 move, struct anonymous_callback* acb);
extern u8 static_on_effect(u8 user, u8 source, u16 move, struct anonymous_callback* acb);
extern void liquid_ooze_on_drain(u8 user, u8 src, u16 move, struct anonymous_callback* acb);
extern u8 cursed_body_on_effect(u8 user, u8 src, u16 move, struct anonymous_callback* acb);
extern u8 weak_armor_on_effect(u8 user, u8 src, u16 move, struct anonymous_callback* acb);
extern u8 justified_on_effect(u8 user, u8 src, u16 move, struct anonymous_callback* acb);
extern u8 stamina_on_effect(u8 user, u8 src, u16 move, struct anonymous_callback* acb);
extern u8 water_compaction_on_effect(u8 user, u8 src, u16 move, struct anonymous_callback* acb);
extern u8 effect_spore_effect(u8 user, u8 src, u16 move, struct anonymous_callback* acb);
extern u8 rough_skin_variations_on_effect(u8 user, u8 src, u16 move, struct anonymous_callback* acb);
extern u8 flamebody_on_effect(u8 user, u8 src, u16 move, struct anonymous_callback* acb);
extern u8 cute_charm_on_effect(u8 user, u8 src, u16 move, struct anonymous_callback* acb);
extern u8 aftermath_on_effect(u8 user, u8 src, u16 move, struct anonymous_callback* acb);
extern u8 poison_touch_on_effect(u8 user, u8 src, u16 move, struct anonymous_callback* acb);
extern u8 mummy_on_effect(u8 user, u8 src, u16 move, struct anonymous_callback* acb);
extern u8 gooey_variations_on_effect(u8 user, u8 src, u16 move, struct anonymous_callback* acb);
extern void pick_pocket_on_secondary(u8 user, u8 src, u16 move, struct anonymous_callback* acb);
extern u8 oblivous_on_tryhit(u8 user, u8 src, u16 move, struct anonymous_callback* acb);
extern u8 oblivious_on_status(u8 user, u8 src, u16 ailment , struct anonymous_callback* acb);
extern u8 own_tempo_on_status(u8 user, u8 src, u16 ailment , struct anonymous_callback* acb);
extern u8 poison_heal_on_effect(u8 user, u8 src, u16 ailment , struct anonymous_callback* acb);
extern u8 sweet_veil_on_status(u8 user, u8 src, u16 ailment , struct anonymous_callback* acb);
extern void prankser_before_turn(u8 user, u8 src, u16 ailment , struct anonymous_callback* acb);
extern void gale_wings_before_turn(u8 user, u8 src, u16 ailment , struct anonymous_callback* acb);
extern bool clear_body_variations_on_stat_boost(u8 user, u8 src, u16 move, struct anonymous_callback* acb);
extern bool keen_eye_on_stat_boost(u8 user, u8 src, u16 move, struct anonymous_callback* acb);
extern bool hyper_cutter_on_stat_boost(u8 user, u8 src, u16 move, struct anonymous_callback* acb);
extern bool white_smoke_on_stat_boost(u8 user, u8 src, u16 move, struct anonymous_callback* acb);
extern bool big_pecks_on_stat_boost(u8 user, u8 src, u16 move, struct anonymous_callback* acb);
extern bool flower_veil_on_stat_boost(u8 user, u8 src, u16 move, struct anonymous_callback* acb);
extern u16 keen_eye_on_stat(u8 user, u8 src, u16 stat_id, struct anonymous_callback* acb);
extern u8 flower_veil_on_status(u8 user, u8 src, u16 ailment, struct anonymous_callback* acb);
extern u16 scrappy_on_effectiveness(u8 target_type, u8 src, u16 move_type, struct anonymous_callback* acb);
extern u8 infiltrator_modify_move(u8 user, u8 src, u16 move, struct anonymous_callback* acb);
extern u8 battle_armor_on_modify_move(u8 user, u8 src, u16 move, struct anonymous_callback* acb);
extern void moxie_on_damage(u8 user, u8 source, u16 move, struct anonymous_callback* acb);
extern void beast_boost_on_damage(u8 user, u8 source, u16 move, struct anonymous_callback* acb);
extern void soul_heart_on_faint(u8 user, u8 source, u16 move, struct anonymous_callback* acb);
extern void receiver_variations_on_faint(u8 user, u8 src, u16 move, struct anonymous_callback* acb);
extern void normalize_before_turn(u8 user, u8 src, u16 move, struct anonymous_callback* acb);
extern void refrigerate_before_turn(u8 user, u8 src, u16 move, struct anonymous_callback* acb);
extern void pixilate_before_turn(u8 user, u8 src, u16 move, struct anonymous_callback* acb);
extern void aerilate_before_turn(u8 user, u8 src, u16 move, struct anonymous_callback* acb);
extern void galvanize_before_turn(u8 user, u8 src, u16 move, struct anonymous_callback* acb);
extern u8 liquid_voice_on_modify_move(u8 user, u8 src, u16 move, struct anonymous_callback* acb);
extern void neuro_force_on_damage(u8 user, u8 src, u16 move, struct anonymous_callback* acb);


#endif /* BATTLE_ABILITIES_TABLE_H_ */
