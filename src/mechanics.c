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
void set_stun(Player *target) {
    target->stun_turns = 1;
    printf("Status: Opponent is stunned! Their next move will be nullified.\n");
}

int get_stun(Player *player) {
    if (player->stun_turns > 0) {
        player->stun_turns = 0;
        return 1;
    }
    return 0;
}


// Apply shackle debuff to target
// Reduces damage dealt on next attack by specified amount
void set_shackle(Player *target) {
    target->shackle_turns = 1;
    target->shackle_damage = 8;
    printf("Status: Opponent is shackled! Damage reduced by %d on next attack.\n", target->shackle_damage);
}

int get_shackle(Player *player) {
    if (player->shackle_turns > 0) {
        player->shackle_turns = 0;
        return player->shackle_damage;
    }
    return 0;
}


//Apply aura buff 50/50 chance to crit dmg
void set_aura(Player *player, int damage) {
        player->aura_active = 1;
        printf("Status: Aura stance activated");
}

float get_aura(Player *player){
    if(player->aura_active > 0){
        player->aura_active = 0;

        if(rand() % 100 < 31) {
            printf("PLUS AURA. 2x dmg buff!");
            return 2.0f;
        }
    }
    return 1.0f;
} 

void check_priority(Card *card1, Card *card2){
    
}

// // APPLYING CARD EFFECT TO TARGET
void card_move(Player *player,Card card) {
    printf("\n --- %s uses %s ---", &player, &card.name);

    
}
