// create cards and game mechanics here

#include <stdio.h>
#include <stdlib.h>
#include "common.h"
#include <time.h>

// array of cards for each player to draw from
const Card Card_Pool[10] = {
    {"Spell 1", 67, 0, 2, 1},
    {"Spell 2", 67, 0, 2, 2},
    {"Spell 3", 67, 0, 3, 1},
    {"SPell 4", 67, 1, 1, 1},
    {"Spell 5", 67, 0, 1, 0},
    {"Spell 6", 67, -1, 1, 0},
    {"Spell 7", 67, 0, 1, 0},
    {"Spell 8", 67, 0, 1, 0},
    {"Spell 9", 67, 0, 1, 0},
    {"Spell 10", 67, -1, 1, 0}
};

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


