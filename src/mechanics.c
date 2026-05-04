// create cards and game mechanics here

#include <stdio.h>
#include <stdlib.h>
#include "common.h"
#include <time.h>

// ALL CARDS/ABILITIES
const Card Card_Pool[11] = { // damage, util (shield/heal), cost parameters

    // ATTACK MOVES (damage dealing, some debuffs and lifesteal)
    {"Laser Beam", 10, 0, 1, 0}, // light atk
    {"Comet", 25, 0, 2, 0}, // heavy atk
    {"Lightning Flash", 8, 0, 1, 1}, // PRIORITY attack. takes queue priority regardless of attack order
    {"Shackle", 5, 1, 1, 0}, // enemy deals -8 damage next atk
    {"Life Drain", 10, 10, 2, 0}, // 10 dmg and 10 heal

    // UTILITY MOVES (defense, healing, buffs, debuffs)
    {"Barrier", 0, 18, 2, 0}, // add 18 shield
    {"Psychic", 2, 0, 1, 0}, // 30% chance to stun enemies. stunned enemies will miss (nullify) their next move
    {"Rejuvenate", 0, 10, 1, 0}, // regen 10hp
    {"Aura Stance", 0, 0, 1, 0}, // next move after aura stance has a 30% chance to deal 2x damage
    {"Arcane Gambit", 10, 20, 1, 0}, // 50/50 chance to heal 20 hp or deal 10 dmg to yourself

    // EMPTY SKIP MOVE TO SKIP TURN 
    {"Skip", 0, 0, 0, 0}
};

void die_with_error(char *error_msg){
    printf("%s", error_msg);
    exit(-1);
}


// Dice roll for checking if who goes first.
int dice_roll(int *p1_roll, int *p2_roll){

    do {
        *p1_roll = (rand() % 6) + 1;
        *p2_roll = (rand() % 6) + 1;
    } while (*p1_roll == *p2_roll); // while no winner, keep rolling

    return (*p1_roll > *p2_roll) ? 1 : 2; // return p1 win else p2 win
}


// the random card giver function to each player at the start of the game and when they draw cards
// mechanics.c

void draw_card(Player *player, int num_cards) { // new rolling system that limits each player to only have two of each card
    for (int i = 0; i < num_cards; i++) {
        if (player->hand_count >= MAX_HAND_SIZE) break;

        Card drawn_card;
        int is_duplicate_limit_reached;
        int attempts = 0;

        // [NEW CODE - Duplicate Limit Logic]
        do {
            is_duplicate_limit_reached = 0;
            int card_index = rand() % 10; // Equal chance for all 10 cards
            drawn_card = Card_Pool[card_index];

            int count = 0;
            for (int j = 0; j < player->hand_count; j++) {
                if (strcmp(player->hand[j].name, drawn_card.name) == 0) {
                    count++;
                }
            }

            if (count >= 2) {
                is_duplicate_limit_reached = 1;
            }
            
            attempts++;
            // Safety break to prevent infinite loops if hand is weirdly full
            if (attempts > 50) break; 

        } while (is_duplicate_limit_reached);

        player->hand[player->hand_count] = drawn_card;
        player->hand_count++;
    }
}

// ------------------ STATUS EFFECTS ------------------

// Execute all card logic
void execute_card(Player *caster, Player *target, Card card, int is_player, char *combat_log) {
    
    char temp[512];
    // Easy naming conventions for calling 
    char *caster_name = is_player ? "P1" : "P2";
    char *target_name = is_player ? "P2" : "P1";

    if (strcmp(card.name, "Skip") == 0) { // SKIP CHECK!!!
    sprintf(temp, ">>> %s decided to skip this turn and conserve energy.\n", caster_name);
    // check for potential overflow
        if (strlen(combat_log) + strlen(temp) < 1023) {
            strcat(combat_log, temp);
        } else {
            // marker that the log was truncated
            strncpy(combat_log + 1010, "...[TRUNCATED]", 14);
        }
    return; // Exit early so no other damage/logic happens
}
        sprintf(temp, "\n--- %s uses [%s]! ---\n", caster_name, card.name);
         if (strlen(combat_log) + strlen(temp) < 1023) {
            strcat(combat_log, temp);
        } else {
            // marker that the log was truncated
            strncpy(combat_log + 1010, "...[TRUNCATED]", 14);
        }

    // -----------------------
    // 1. PRE-ATTACK CHECKS 
    // -----------------------
    if (caster->stun_turns > 0) {
            sprintf(temp, "Status: %s is STUNNED! The move is nullified.\n", caster_name);
             if (strlen(combat_log) + strlen(temp) < 1023) {
            strcat(combat_log, temp);
        } else {
            // marker that the log was truncated
            strncpy(combat_log + 1010, "...[TRUNCATED]", 14);
        }
        caster->stun_turns = 0; // Remove stun
        return; // EXIT FUNCTION: Card does nothing
    }

    // --------------------
    // 2. APPLY UTILITY 
    // --------------------
    if (card.utility > 0) {
        if (strcmp(card.name, "Barrier") == 0) {
            caster->shield += card.utility;
                sprintf(temp, "Status: %s gains %d Shield!\n", caster_name, card.utility);
                 if (strlen(combat_log) + strlen(temp) < 1023) {
                    strcat(combat_log, temp);
                } else {
                    // marker that the log was truncated
                    strncpy(combat_log + 1010, "...[TRUNCATED]", 14);
                }
        } 
        else if (strcmp(card.name, "Rejuvenate") == 0 || strcmp(card.name, "Life Drain") == 0) {
            caster->hp += card.utility;
            if (caster->hp > MAX_HP) caster->hp = MAX_HP; // Prevent overheal
                sprintf(temp, "Status: %s heals for %d HP!\n", caster_name, card.utility);
                        if (strlen(combat_log) + strlen(temp) < 1023) {
                    strcat(combat_log, temp);
                } else {
                    // marker that the log was truncated
                    strncpy(combat_log + 1010, "...[TRUNCATED]", 14);
                }
    }
}

    // ------------------------------
    // 3. CALCULATE AND APPLY DAMAGE
    // ------------------------------
    if (card.damage > 0) {
        float final_damage = card.damage;

        // Apply Aura Buff (if active)
        if (caster->aura_active > 0) {
            caster->aura_active = 0; 
            if (rand() % 100 < 31) { // 30% chance roughly
                    sprintf(temp, "PLUS AURA! 2x Damage buff triggered!\n");
                if (strlen(combat_log) + strlen(temp) < 1023) {
                    strcat(combat_log, temp);
                } else {
                    // marker that the log was truncated
                    strncpy(combat_log + 1010, "...[TRUNCATED]", 14);
                }
                final_damage *= 2.0f;
            }
        
        }

        // Apply Shackle Debuff (if active)
        if (caster->shackle_turns > 0) {
                sprintf(temp, "Status: %s is Shackled! Damage reduced by %d.\n", caster_name, caster->shackle_damage);
                 if (strlen(combat_log) + strlen(temp) < 1023) {
                    strcat(combat_log, temp);
                } else {
                    // marker that the log was truncated
                    strncpy(combat_log + 1010, "...[TRUNCATED]", 14);
                }
            final_damage -= caster->shackle_damage;
            caster->shackle_turns = 0; // consume shackle
            if (final_damage < 0) final_damage = 0; // No negative damage
        }

        // Apply Damage to Shield first, then HP
        int dmg_int = (int)final_damage;
        if (dmg_int > 0) {
            if (target->shield > 0) {
                if (target->shield >= dmg_int) {
                    target->shield -= dmg_int;
                        sprintf(temp, "%s's Shield absorbed %d damage! (Shield left: %d)\n", target_name, dmg_int, target->shield);
                    if (strlen(combat_log) + strlen(temp) < 1023) {
                        strcat(combat_log, temp);
                    } else {
                        // marker that the log was truncated
                        strncpy(combat_log + 1010, "...[TRUNCATED]", 14);
                    }
                    dmg_int = 0; // Damage fully absorbed
                } else {
                        sprintf(temp, "%s's Shield absorbed %d damage, but broke!\n", target_name, target->shield);
                         if (strlen(combat_log) + strlen(temp) < 1023) {
                        strcat(combat_log, temp);
                    } else {
                        // marker that the log was truncated
                        strncpy(combat_log + 1010, "...[TRUNCATED]", 14);
                    }
                    dmg_int -= target->shield;
                    target->shield = 0;
                }
            }
            
            // Remaining damage hits HP
            if (dmg_int > 0) {
                target->hp -= dmg_int;
                    sprintf(temp, "%s takes %d damage!\n", target_name, dmg_int);
                     if (strlen(combat_log) + strlen(temp) < 1023) {
                    strcat(combat_log, temp);
                } else {
                    // marker that the log was truncated
                    strncpy(combat_log + 1010, "...[TRUNCATED]", 14);
                }
            }
        }
    }

    // ------------------------------------------
    // 4. APPLY SPECIAL STATUS EFFECTS TO TARGET
    // --------------------------------------------
    if (strcmp(card.name, "Psychic") == 0) {
        if (rand() % 100 < 31) { // 30% chance
            target->stun_turns = 1;
                sprintf(temp, "Status: %s is STUNNED!\n", target_name);
                 if (strlen(combat_log) + strlen(temp) < 1023) {
                strcat(combat_log, temp);
            } else {
                // marker that the log was truncated
                strncpy(combat_log + 1010, "...[TRUNCATED]", 14);
            }
        }
    } 
    else if (strcmp(card.name, "Shackle") == 0) {
        target->shackle_turns = 1;
        target->shackle_damage = 8;
            sprintf(temp, "Status: %s is SHACKLED!\n", target_name);
            if (strlen(combat_log) + strlen(temp) < 1023) {
                strcat(combat_log, temp);
            } else {
                // marker that the log was truncated
                strncpy(combat_log + 1010, "...[TRUNCATED]", 14);
            }
    }

    else if (strcmp(card.name, "Aura Stance") == 0) {
        caster->aura_active = 1;
            sprintf(temp, "Status: %s enters Aura Stance!\n", caster_name);
            if (strlen(combat_log) + strlen(temp) < 1023) {
                strcat(combat_log, temp);
            } else {
                // marker that the log was truncated
                strncpy(combat_log + 1010, "...[TRUNCATED]", 14);
            }
    }

    else if (strcmp(card.name, "Arcane Gambit") == 0) {
        if (rand() % 100 < 50) {
            caster->hp += 20;
            if (caster->hp > MAX_HP) caster->hp = MAX_HP;
                sprintf(temp, "Gambit Success: %s heals 20 HP!\n", caster_name);
                if (strlen(combat_log) + strlen(temp) < 1023) {
                strcat(combat_log, temp);
            } else {
                // marker that the log was truncated
                strncpy(combat_log + 1010, "...[TRUNCATED]", 14);
            }

        } else {
            caster->hp -= 10;
                sprintf(temp, "Gambit Fail: %s takes 10 recoil damage!\n", caster_name);
                if (strlen(combat_log) + strlen(temp) < 1023) {
                strcat(combat_log, temp);
            } else {
                // marker that the log was truncated
                strncpy(combat_log + 1010, "...[TRUNCATED]", 14);
            }
        }
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
        return; // <--- FIX: Prevents logic from continuing if full
    }

    if(is_empty(q)){
        q->front = q->rear = 0;
    }
    else {
        q->rear = (q->rear + 1) % q->size;
    }

    q->queue[q->rear] = data;
 } // <--- SYNTAX FIX: This brace closes the enqueue function properly

Action dequeue(ActionQueue *q){
    Action empty = q->queue[q->front];
    
    if(q->front == q->rear) {
        q->front = q->rear = -1;
    } else {
        q->front = (q->front + 1) % q->size;
    }
    return empty;
}

