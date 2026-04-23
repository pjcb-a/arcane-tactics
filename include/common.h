// HEADER FILE like a global blueprint for the server and client 
// that updates as the game progresses. 

#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_HP 100
#define START_ENERGY 2
#define START_HAND_SIZE 3

typedef struct {
    char name[20];
    int damage;
    int utility;
    int cost;
} Card;

typedef struct {
    int hp;
    int energy;
    Card hand[3];
    int hand_count;
} Player;

// Game state after each turn
typedef struct {
    Player p1_status; // Server 
    Player p2_status; // Client 
    char message[256]; // "Combat results", "Your turn", etc.
} GameState;

void die_with_error(char *error_msg);
void draw_card(Player *player, int num_cards);

#endif