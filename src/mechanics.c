// create cards and game mechanics here

#include <stdio.h>
#include <stdlib.h>
#include "common.h"
#include <time.h>

// ALL CARDS/ABILITIES
const Card Card_Pool[10] = { // damage, util (shield/heal), cost parameters

    // ATTACK MOVES (damage dealing, some debuffs and lifesteal)
    {"Laser Beam", 10, 0, 1}, // light atk
    {"Comet", 25, 0, 2}, // heavy atk
    {"Lightning Flash", 8, 0, 1}, // PRIORITY attack. takes queue priority regardless of attack order
    {"Shackle", 5, 1, 1}, // enemy deals -8 damage next atk
    {"Life Drain", 10, 10, 2}, // 10 dmg and 10 heal

    // UTILITY MOVES (defense, healing, buffs, debuffs)
    {"Barrier", 0, 18, 2}, // add 18 shield
    {"Psychic", 2, 0, 1}, // 40% chance to Confus
    {"Rejuvenate", 0, 10, 1}, 
    {"Aura Stance", 0, 0, 1},
    {"Arcane Gambit", 10, 20, 1}
};

// to add ni xian: status effects (stun and shackle)

// just error handling functions...
void die_with_error(char *error_msg){
    printf("%s", error_msg);
    exit(-1);
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


