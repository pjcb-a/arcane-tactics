// HEADER FILE like a global blueprint for the server and client 
// that updates as the game progresses. 

#ifndef COMMON_H
#define COMMON_H

#define MAX_HP 100
#define START_ENERGY 2
#define START_HAND_SIZE 3

typedef struct {
    char name[20];
    int damage;
    int defense;
    int cost;
} Card;

typedef struct {
    int hp;
    int energy;
    Card hand[10];
    int hand_count;
} Player;

// This is what the Server sends to the Client to update their screen
typedef struct {
    Player p1_status; // Server (hidden hand)
    Player p2_status; // Client (visible hand)
    char message[256]; // "Combat results", "Your turn", etc.
} GameState;

#endif