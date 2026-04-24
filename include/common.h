// HEADER FILE like a global blueprint for the server and client 
// that updates as the game progresses. 

#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_HP 100
#define START_ENERGY 2
#define MAX_HAND_SIZE 10
#define START_HAND_SIZE 3

typedef struct {
    char name[20];
    int damage;
    int utility;
    int cost;
    int priority;
} Card;

typedef struct {
    int hp;
    int energy;
    Card hand[MAX_HAND_SIZE];
    int hand_count;
    int shield;
    int aura_active; 
    int stun_turns; // > 0 then stun
    int shackle_damage; // > 0 then dmg reduced
    int shackle_turns; // how many turns
} Player;

// Game state after each turn
typedef struct {
    Player p1_status; // Server
    Player p2_status; // Client
    char message[256]; // "Combat results", "Your turn", etc.
    int p1_roll;
    int p2_roll;
} GameState;

void die_with_error(char *error_msg);
void draw_card(Player *player, int num_cards);
void apply_card_effect(Player *attacker, Player *defender, Card *card); // to apply stun and 
int dice_roll(int *p1_roll, int *p2_roll);

#endif