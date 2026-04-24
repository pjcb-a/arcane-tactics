// create cards and game mechanics here

#include <stdio.h>
#include <stdlib.h>
#include "common.h"
#include <time.h>

// ALL CARDS/ABILITIES
const Card Card_Pool[10] = { // damage, util (shield/heal), cost parameters

    // ATTACK MOVES (damage dealing, some debuffs and lifesteal)
    {"Laser Beam", 10, 0, 1, 0}, // light atk
    {"Comet", 25, 0, 2, 0}, // heavy atk
    {"Lightning Flash", 8, 0, 1, 1}, // PRIORITY attack. takes queue priority regardless of attack order
    {"Shackle", 5, 1, 1, 0}, // enemy deals -8 damage next atk
    {"Life Drain", 10, 10, 2, 0}, // 10 dmg and 10 heal

    // UTILITY MOVES (defense, healing, buffs, debuffs)
    {"Barrier", 0, 18, 2, 0}, // add 18 shield
    {"Psychic", 2, 0, 1, 0}, // 40% chance to stun enemies. stunned enemies will miss (nullify) their next move
    {"Rejuvenate", 0, 10, 1, 0}, // regen 10hp
    {"Aura Stance", 0, 0, 1, 0}, // next move after aura stance has a 30% chance to deal 2x damage
    {"Arcane Gambit", 10, 20, 1, 0} // 50/50 chance to heal 20 hp or deal 10 dmg to yourself
};

// to add ni xian: status effects (stun and shackle)

// Just error handling functions...
void die_with_error(char *error_msg){
    printf("%s", error_msg);
    exit(-1);
}


// Dice roll for checking if who goes first.
int dice_roll(int *p1_roll, int *p2_roll){

    do {
        *p1_roll = (rand() % 6) + 1;
        *p2_roll = (rand() % 6) + 1;
    } while (*p1_roll == *p2_roll);

    return (*p1_roll > *p2_roll) ? 1 : 2;
}


// the random card giver function to each player at the start of the game and when they draw cards
void draw_card(Player *player, int num_cards) {

    // error handling for hand count -- segmentation fault out of bounds --
    if(player->hand_count < 0 || player->hand_count > MAX_HAND_SIZE) {
        player->hand_count = 0; // reset hand count if out of bounds
    }

    for(int i = 0; i < num_cards; i++) {
        if(player->hand_count >= MAX_HAND_SIZE) {
            printf("Hand is full! Cannot draw more cards.\n");
            return;
        }
        int rand_index = rand() % 10; // random index for card pool

        player->hand[player->hand_count] = Card_Pool[rand_index];
        player->hand_count++;
    }
}

// ==================== STATUS EFFECTS ====================

// Apply stun to target player
void apply_stun(Player *target) {
    // TODO: Set stun_turns to 1
    target->stun_turns = 1;
    printf("Status: Opponent is stunned! Their next move will be nullified.\n");
}

// Check if player is stunned
// Returns: 1 if stunned, 0 if not
int check_stun(Player *player) {
    if (player->stun_turns > 0) {
        return 1;
    }
    return 0;
}

// Apply shackle debuff to target
// Reduces damage dealt on next attack by specified amount
void apply_shackle(Player *target, int reduction) {
    target->shackle_damage = reduction;
    target->shackle_turns = 1;
    printf("Status: Opponent is shackled! Damage reduced by %d on next attack.\n", reduction);
}

// Get the current shackle damage reduction
// Returns: reduction amount if active, 0 if not
int get_shackle_reduction(Player *player) {
    if (player->shackle_turns > 0) {
        return player->shackle_damage;
    }
    return 0;
}

// Update status effects at end of turn
// Decrements counters and clears expired effects
void update_status_effects(Player *player) {
    // Decrement stun counter
    if (player->stun_turns > 0) {
        player->stun_turns--;
        if (player->stun_turns == 0) {
            printf("Status: Stun has worn off.\n");
        }
    }
    
    // Decrement shackle counter
    if (player->shackle_turns > 0) {
        player->shackle_turns--;
        if (player->shackle_turns == 0) {
            player->shackle_damage = 0;
            printf("Status: Shackle has worn off.\n");
        }
    }
}

// APPLYING CARD EFFECT TO TARGET
void apply_card_effect(Player *attacker, Player *defender, Card *card) {
    
    // Check if defender is stunned
    if (check_stun(defender) == 1) {
        printf("%s was used but opponent is stunned! Move nullified.\n", card->name);
        // Clear stun after it's used
        defender->stun_turns = 0;
        return;
    }
    
    // Get shackle reduction
    int shackle_reduction = get_shackle_reduction(defender);
    
    // ========== ATTACK CARDS ==========
    
    if (strcmp(card->name, "Laser Beam") == 0) {
        int damage = card->damage - shackle_reduction;
        if (damage < 0) damage = 0;
        defender->hp -= damage;
        printf("Laser Beam deals %d damage!\n", damage);
    }
    else if (strcmp(card->name, "Comet") == 0) {
        int damage = card->damage - shackle_reduction;
        if (damage < 0) damage = 0;
        defender->hp -= damage;
        printf("Comet deals %d damage!\n", damage);
    }
    else if (strcmp(card->name, "Lightning Flash") == 0) {
        int damage = card->damage - shackle_reduction;
        if (damage < 0) damage = 0;
        defender->hp -= damage;
        printf("Lightning Flash deals %d damage!\n", damage);
    }
    else if (strcmp(card->name, "Shackle") == 0) {
        // Deal damage
        int damage = card->damage - shackle_reduction;
        if (damage < 0) damage = 0;
        defender->hp -= damage;
        // Apply shackle debuff
        apply_shackle(defender, 8);
        printf("Shackle deals %d damage and reduces next attack by 8!\n", damage);
    }
    else if (strcmp(card->name, "Life Drain") == 0) {
        int damage = card->damage - shackle_reduction;
        if (damage < 0) damage = 0;
        defender->hp -= damage;
        attacker->hp += card->utility;  // Heal self
        printf("Life Drain deals %d damage and heals %d HP!\n", damage, card->utility);
    }
    
    // ========== UTILITY CARDS ==========
    
    else if (strcmp(card->name, "Barrier") == 0) {
        // Note: You need to add shield field to Player struct
        // attacker->shield += card->utility;
        attacker->hp += card->utility;  // Using HP as temporary shield
        printf("Barrier adds %d shield!\n", card->utility);
    }
    else if (strcmp(card->name, "Psychic") == 0) {
        // 40% chance to stun
        if (rand() % 100 < 40) {
            apply_stun(defender);
            printf("Psychic STUNS the opponent! (40%% chance)\n");
        } else {
            printf("Psychic failed to stun.\n");
        }
    }
    else if (strcmp(card->name, "Rejuvenate") == 0) {
        attacker->hp += card->utility;
        if (attacker->hp > MAX_HP) attacker->hp = MAX_HP;
        printf("Rejuvenate heals %d HP!\n", card->utility);
    }
    else if (strcmp(card->name, "Aura Stance") == 0) {
        // Note: You need to add aura_active field to track this
        // attacker->aura_active = 1;
        printf("Aura Stance activated! Next attack has 30%% chance for 2x damage.\n");
    }
    else if (strcmp(card->name, "Arcane Gambit") == 0) {
        // 50/50 chance
        if (rand() % 2 == 0) {
            // Heal self
            attacker->hp += card->utility;
            if (attacker->hp > MAX_HP) attacker->hp = MAX_HP;
            printf("Arcane Gambit: SUCCESS! Healed %d HP.\n", card->utility);
        } else {
            // Damage self
            attacker->hp -= card->damage;
            printf("Arcane Gambit: FAILED! Took %d self damage.\n", card->damage);
        }
    }
    
    // Clear shackle after it blocks one attack
    if (shackle_reduction > 0) {
        defender->shackle_turns = 0;
        defender->shackle_damage = 0;
    }
}
