// HEADER FILE like a global blueprint for the server and client 
// that updates as the game progresses. 

#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define RESET   "\033[0m"
#define RED     "\033[1;31m"
#define GREEN   "\033[1;32m"
#define YELLOW  "\033[1;33m"
#define BLUE    "\033[1;34m"
#define CYAN    "\033[1;36m"
#define WHITE   "\033[1;37m"
#define MAGENTA "\033[1;35m"
#define BOLD    "\033[1m"

// Semantic Colors
#define COLOR_DAMAGE RED
#define COLOR_HEAL   GREEN
#define COLOR_ENERGY BLUE
#define COLOR_PRIO   YELLOW
#define COLOR_NAME   CYAN
#define COLOR_INFO   WHITE
#define COLOR_MSG    MAGENTA

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
    char desc[128];
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
    char message[2048]; // game messages increased from 512
    int p1_roll;
    int p2_roll;
    int turn_winner;
    int deck[30];
    int deck_ptr;
} GameState;

// Queue logic for card prio and dmg distribution with DSA source code

// Move to input to the Queue
typedef struct {
    int player_id;
    Card card;
} Action;

typedef struct {
    Action queue[10];
    int front;
    int rear;
    int size;
} ActionQueue;


void die_with_error(char *error_msg);
void print_welcome_banner();
void display_card_glossary();
void typewriter(const char *text, int delay_ms);
void shuffle_deck(GameState *game);
void draw_card(Player *player, GameState *game, int num_cards);
void execute_card(Player *caster, Player *target, Card card, int is_player, char *combat_log);
void card_move(Player *player, Card card);
void remove_card(Player *player, int card_index);
int dice_roll(int *p1_roll, int *p2_roll);
void print_stat_bars(Player *p, const char* label);
void print_victory_screen();
void print_defeat_screen();

void init_queue(ActionQueue *q, int size);
int is_empty(ActionQueue *q);
int is_full(ActionQueue *q);
void enqueue(ActionQueue *q, Action data);
Action dequeue(ActionQueue *q);
void apply_perspective(const char *log, char *out, int is_p1);
void get_ui_elements(Player *p, char *hp_bar, char *status_str);
#endif