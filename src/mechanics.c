// create cards and game mechanics here

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "common.h"
#include <time.h>
#include <ctype.h>


// ALL CARDS/ABILITIES
const Card Card_Pool[10] = { // damage, util (shield/heal), cost parameters

    // ATTACK MOVES (damage dealing, some debuffs and lifesteal)
    {"Laser Beam", 15, 0, 1, 0, "Light attack dealing 15 damage." }, // light atk
    {"Comet", 35, 0, 2, 0, "Heavy attack dealing 35 damage." }, // heavy atk
    {"Lightning Flash", 16, 0, 1, 1, "Quick strike dealing 16 damage. Hits first." }, // PRIORITY attack. takes queue priority regardless of attack order
    {"Shackle", 8, 1, 1, 0, "Deals 8 damage and reduces enemy's next attack by 8." }, // enemy deals -8 damage next atk
    {"Life Drain", 15, 10, 2, 0, "Vampiric strike. Deals 15 damage and heals 10 HP." }, // 15 dmg and 10 heal

    // UTILITY MOVES (defense, healing, buffs, debuffs)
    {"Barrier", 0, 18, 2, 1, "Provides 18 Shield to block damage. Priority move." }, // add 18 shield
    {"Psychic", 5, 0, 1, 0, "Deals 5 damage. 30% chance to stun the enemy." }, // 30% chance to stun enemies. stunned enemies will miss (nullify) their next move
    {"Rejuvenate", 0, 10, 1, 0, "Restores 10 HP." }, // regen 10hp
    {"Aura Stance", 0, 0, 1, 0, "Focuses energy. Next move deals double damage." }, // next move after aura stance has a guaranteed chance to deal 2x damage
    {"Arcane Gambit", 10, 20, 1, 0, "Deals 10 damage. 50/50 chance to heal 20 hp or take 10 recoil." }, // 50/50 chance to heal 20 hp or deal 10 dmg to yourself and the enemy

};

void die_with_error(char *error_msg){
    printf("%s", error_msg);
    exit(-1);
}


// UI DESIGN
void clear_screen() {
    printf("\033[2J\033[H"); // Clears the terminal and moves cursor to top-left
}

void print_welcome_banner() {
    clear_screen();
    printf("=======================================================================\n");
    printf("      ___                               _____           _   _            \n");
    printf("     / _ \\                            |_   _|         | | (_)          \n");
    printf("    / /_\\ \\_ __ ___ __ _ _ __   ___    | | __ _  ___| |_ _  ___  ___  \n");
    printf("    |  _  | '__/ __/ _` | '_ \\ / _ \\   | |/ _` |/ __| __| |/ __ / __| \n");
    printf("    | | | | | | (_| (_| | | | || __/     | | (_| | (__| |_| | (__ \\__ \\ \n");
    printf(" \\_| |_/_|  \\___\\__,_|_| |_|\\___| \\_/\\__,_|\\___|\\__|_|\\___|___/\n");
    printf("=======================================================================\n");
    printf("                      Waiting for opponent...\n");
    printf("=======================================================================\n\n");
}


void typewriter(const char *text, int delay_ms) {
    if (text == NULL) return;
    for (int i = 0; text[i] != '\0'; i++) {
        printf("%c", text[i]);
        fflush(stdout); // Forces the letter to print immediately
        usleep(delay_ms * 1000);
    }
    printf("\n");
}

void display_card_glossary() {
    printf("\n--- Arcane Tactics : Spellbook ---\n");
    for (int i = 0; i < 10; i++) {
        char buffer[256];
        sprintf(buffer, "[%s] Cost: %d | Prio: %d\n >> %s", 
                Card_Pool[i].name, Card_Pool[i].cost, 
                Card_Pool[i].priority, Card_Pool[i].desc);
        typewriter(buffer, 50);
        printf("\n");
    }
    printf("----------------------------------\n\n");
}


// Dice roll for checking if who goes first.
int dice_roll(int *p1_roll, int *p2_roll){

    do {
        *p1_roll = (rand() % 6) + 1;
        *p2_roll = (rand() % 6) + 1;
    } while (*p1_roll == *p2_roll); // while no winner, keep rolling

    return (*p1_roll > *p2_roll) ? 1 : 2; // return p1 win else p2 win
}


// NEW UI FOR HP AND STAT BARS
void print_stat_bars(Player *p, const char* label) {
    printf("%s %-10s %s\n", BOLD, label, RESET);
    
    // HP Bar: Green if high, Yellow if mid, Red if low
    printf("HP:  [");
    if (p->hp > 60) printf(GREEN);
    else if (p->hp > 25) printf(YELLOW);
    else printf(RED);

    int bars = (p->hp * 20) / MAX_HP; 
    if (bars < 0) bars = 0; // Prevent negative bars if HP is negative
    for (int i = 0; i < 20; i++) {
        if (i < bars) printf("■");
        else printf(" ");
    }
    printf(RESET "] %d/%d\n", p->hp, MAX_HP);

    // Shield: Only shows if active (Cyan)
    if (p->shield > 0) {
        printf("SHD: " CYAN "[");
        for(int i=0; i < (p->shield/5); i++) printf("≡");
        printf("] (+%d)" RESET "\n", p->shield);
    }

    // Energy: Blue dots
    printf("NRG: " BLUE);
    for (int i = 0; i < p->energy; i++) printf("♦ ");
    printf(RESET "(%d/10)\n", p->energy);

    // Status Effects
    if (p->stun_turns > 0) printf(YELLOW " [!] STUNNED " RESET);
    if (p->shackle_turns > 0) printf(RED " [!] SHACKLED " RESET);
    if (p->aura_active) printf(CYAN " [!] AURA ACTIVE " RESET);
    printf("\n");
}

// the random card giver/shuffler function to each player at the start of the game and when they draw cards
// mechanics.c

void shuffle_deck(GameState *game) {
    for (int i = 0; i < 30; i++) {
        game->deck[i] = i % 10; 
    }

    // optimized card shuffle wherein a card can be shuffled 3 times only
    for (int i = 29; i > 0; i--) {
        int j = rand() % (i + 1);
        // Swap deck[i] with deck[j]
        int temp = game->deck[i];
        game->deck[i] = game->deck[j];
        game->deck[j] = temp;
    }
    game->deck_ptr = 0; // Reset pointer to the top of the deck
}

void draw_card(Player *player, GameState *game, int num_cards) {
    for (int i = 0; i < num_cards; i++) {
        if (player->hand_count < MAX_HAND_SIZE) {
            // Simply take the card the deck_ptr is pointing to
            int card_id = game->deck[game->deck_ptr];
            player->hand[player->hand_count] = Card_Pool[card_id];
            player->hand_count++;
            
            game->deck_ptr++;
            
            // If we run out of cards (30 cards drawn), reshuffle
            if (game->deck_ptr >= 30) {
                shuffle_deck(game);
            }
        }
    }
}

// Helper Function for overflow checks
static void safe_append_log(char *combat_log, const char *new_text) {
    if (strlen(combat_log) + strlen(new_text) < 1023) {
        strcat(combat_log, new_text);
    } else {
        // Only add truncated marker once
        if (strstr(combat_log, "[TRUNCATED]") == NULL) {
            strncpy(combat_log + 1010, "...[TRUNCATED]", 14);
        }
    }
}


void apply_perspective(const char *log, char *out, int is_p1) {
    const char *self_tag = is_p1 ? "P1" : "P2";
    const char *enemy_tag = is_p1 ? "P2" : "P1";
    int out_i = 0, len = strlen(log);

    for (int i = 0; i < len && out_i < 1020; i++) {
        // Check for "P1's" or "P2's" (Possessive)
        if (strncmp(log + i, self_tag, 2) == 0 && log[i+2] == '\'' && log[i+3] == 's') {
            strcpy(out + out_i, "Your");
            out_i += 4; i += 3; continue;
        }
        if (strncmp(log + i, enemy_tag, 2) == 0 && log[i+2] == '\'' && log[i+3] == 's') {
            strcpy(out + out_i, "Opponent's");
            out_i += 10; i += 3; continue;
        }

        // Check for "P1" or "P2" (Subject)
        if (strncmp(log + i, self_tag, 2) == 0) {
            strcpy(out + out_i, "You");
            out_i += 3; i += 1; continue;
        }
        if (strncmp(log + i, enemy_tag, 2) == 0) {
            strcpy(out + out_i, "Opponent");
            out_i += 8; i += 1; continue;
        }

        out[out_i++] = log[i];
    }
    out[out_i] = '\0';
}

// ------------------ STATUS EFFECTS ------------------

// Execute all card logic
void execute_card(Player *caster, Player *target, Card card, int is_player, char *combat_log) {
    char temp[256];
    char *caster_name = is_player ? "P1" : "P2";
    char *target_name = is_player ? "P2" : "P1";

    // Handle Skip early
    if (strcmp(card.name, "Skip") == 0) {
        sprintf(temp, ">>> %s decided to skip this turn and conserve energy.\n", caster_name);
        safe_append_log(combat_log, temp);
        return;
    }

    sprintf(temp, "\n--- " BOLD "%s" RESET " used " BLUE "[%s]!" RESET "---\n", caster_name, card.name);
    safe_append_log(combat_log, temp);

    // 1. PRE-ATTACK CHECKS 
    if (caster->stun_turns > 0) {
        sprintf(temp, "Status: %s cannot move - STUNNED! The move is nullified.\n", caster_name);
        safe_append_log(combat_log, temp);
        caster->stun_turns = 0; 
        return; 
    }

    // 2. APPLY UTILITY (Healing & Shields)
    if (strcmp(card.name, "Barrier") == 0) {
        caster->shield += card.utility;
        sprintf(temp, "Status: %s gained %d Shield!\n", caster_name, card.utility);
        safe_append_log(combat_log, temp);
    }
    
    if (strcmp(card.name, "Rejuvenate") == 0 || strcmp(card.name, "Life Drain") == 0) {
        caster->hp += card.utility;
        if (caster->hp > MAX_HP) caster->hp = MAX_HP;
        sprintf(temp, "Status: %s healed for %d HP!\n", caster_name, card.utility);
        safe_append_log(combat_log, temp);
    }

    // 3. CALCULATE AND APPLY DAMAGE
    if (card.damage > 0) {
        float final_damage = card.damage;

        if (caster->shackle_turns > 0) {
            sprintf(temp, "Status: %s cannot attack freely - Shackled! %s's damage is reduced by %d.\n", caster_name, caster_name, caster->shackle_damage);
            final_damage -= caster->shackle_damage;
            safe_append_log(combat_log, temp);
            caster->shackle_turns = 0;

            if (final_damage < 0) {
                final_damage = 0;
            }
        }

        if (caster->aura_active > 0) { 
            safe_append_log(combat_log, "PLUS AURA! 2x Damage buff triggered!\n");
            final_damage *= 2.0f;
            caster->aura_active = 0;
        }
        
        int dmg_int = (int)final_damage;
        if (dmg_int > 0) {
            
            if (target->shield > 0) {
                if (target->shield >= dmg_int) {
                    target->shield -= dmg_int;
                    sprintf(temp, "%s's Shield absorbed %d damage! (Remaining: %d)\n", target_name, dmg_int, target->shield);
                    dmg_int = 0;
                } else {
                    sprintf(temp, "%s's Shield broke! %d damage bypassed.\n", target_name, target->shield);
                    dmg_int -= target->shield;
                    target->shield = 0;
                }
                safe_append_log(combat_log, temp);
            }
            
            if (dmg_int > 0) {
                target->hp -= dmg_int;
                sprintf(temp, BOLD " %s " RESET "took" RED " %d " RESET "damage!\n", target_name, dmg_int);
                safe_append_log(combat_log, temp);
            }
        }
    }
    
    // 4. SPECIAL STATUS EFFECTS
    if (strcmp(card.name, "Psychic") == 0 && (rand() % 100 < 31)) {
        target->stun_turns = 1;
        sprintf(temp, "Status: %s cannot move - STUNNED! The move is nullified.\n", target_name);
        safe_append_log(combat_log, temp);
    } 
    else if (strcmp(card.name, "Shackle") == 0) {
        target->shackle_turns = 1;
        target->shackle_damage = 8;
        sprintf(temp, "Status: %s cannot attack freely - Shackled! Damage reduced by %d.\n", target_name, target->shackle_damage);
        safe_append_log(combat_log, temp);
    }
    else if (strcmp(card.name, "Aura Stance") == 0) {
        caster->aura_active = 1;
        sprintf(temp, "Status: %s entered Aura Stance!\n", caster_name);
        safe_append_log(combat_log, temp);
    }
    else if (strcmp(card.name, "Arcane Gambit") == 0) {
        if (rand() % 100 < 50) {
            caster->hp += 20;
            if (caster->hp > MAX_HP) caster->hp = MAX_HP;
            sprintf(temp, "Gambit Success: %s healed for 20 HP!\n", caster_name);
        } else {
            caster->hp -= 10;
            sprintf(temp, "Gambit Fail: %s took 10 recoil damage!\n", caster_name);
        }
        safe_append_log(combat_log, temp);
    }
}


// Removes a card from hand, shifts remaining cards left
void remove_card(Player *player, int card_index) {
    if (card_index < 0 || card_index >= player->hand_count) {
        return;
    }
    for (int i = card_index; i < player->hand_count - 1; i++) {
        player->hand[i] = player->hand[i + 1];
    }
    player->hand_count--;
}

// ------------------= CARD QUEUE LOGIC ----------------------

void init_queue(ActionQueue *queue, int size) {
    queue->size = size;
    queue->front = queue->rear = -1;
}

int is_empty(ActionQueue *q) {
    if (q->front == -1 && q->rear == -1) {
        return 1;
    }
    return 0;
}

int is_full(ActionQueue *q) {
    if ((q->rear + 1) % q->size == q->front) {
        return 1;
    }
    return 0;
}

void enqueue(ActionQueue *q, Action data){
    if(is_full(q)) {
        printf("Queue is Full\n");
        return; 
    }

    if(is_empty(q)){
        q->front = q->rear = 0;
    }
    else {
        q->rear = (q->rear + 1) % q->size;
    }

    q->queue[q->rear] = data;
 } 

Action dequeue(ActionQueue *q){
    Action empty = q->queue[q->front];
    
    if(q->front == q->rear) {
        q->front = q->rear = -1;
    } else {
        q->front = (q->front + 1) % q->size;
    }
    return empty;
}

void print_victory_screen() {
    clear_screen();
    printf(CYAN BOLD);
    printf("==========================================================\n");
    printf("  _    _  _____  _____  _______  _____  ______ __   __\n");
    printf(" | |  | ||_   _|/ ____||__   __|/ __  \\|  __  \\\\ \\ / /\n");
    printf(" | |  | |  | | | |        | |  | |  | || |__) | \\ V / \n");
    printf(" | |/\\| |  | | | |        | |  | |  | ||  _  /   > <  \n");
    printf(" \\  /\\  / _| |_| |____    | |  | |__| || | \\ \\  / . \\ \n");
    printf("  \\/  \\/ |_____|\\_____|   |_|   \\____/ |_|  \\_\\/_/ \\_\\\n");
    printf("==========================================================\n");
    printf("                YOU DEFEATED THE OPPONENT!                \n");
    printf("==========================================================\n" RESET);
}

void print_defeat_screen() {
    clear_screen();
    printf(RED BOLD);
    printf("==========================================================\n");
    printf("  _____   ______  ______  ______   ___   _______  \n");
    printf(" |  __ \\ |  ____||  ____||  ____| / _ \\ |__   __|\n");
    printf(" | |  | || |__   | |__   | |__   / /_\\ \\   | |   \n");
    printf(" | |  | ||  __|  |  __|  |  __|  |  _  |   | |   \n");
    printf(" | |__| || |____ | |     | |____ | | | |   | |   \n");
    printf(" |_____/ |______||_|     |______||_| |_|   |_|   \n");
    printf("==========================================================\n");
    printf("                YOU HAVE BEEN STRUCK DOWN                 \n");
    printf("==========================================================\n" RESET);
}

