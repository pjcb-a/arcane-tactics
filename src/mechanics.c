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


// // APPLYING CARD EFFECT TO TARGET



// ===================== CARD QUEUE LOGIC ==========================

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
    }

    if(isEmpty(q)){
        q->front = q->rear = 0;
    }
    else {
        q->rear = (q->rear + 1) % q->size;
    }

    q->queue[q->rear] = data;
}

Action dequeue(ActionQueue *q){
    Action empty = {0};
    if(isEmpty(q)){
        printf("Queue is already empty\n");
        return empty;
    }

    Action data = q->queue[q->front];
    
    if (q->front == q->rear){
        q->front = q->rear = -1;
    }
    else {
        q->front = (q->front + 1) % q->size;
    }

    return data;
}

// class Queue {
//     string *queue;
//     int front, rear, size;

//     public:
//         Queue(int);
//         bool isEmpty();
//         bool isFull();
//         void enqueue(string);
//         void dequeue();
//         string peek();
//         void display();
//     };

// Queue::Queue(int s){
//     size = s;
//     front = rear = -1;
//     queue = new string[s];
// }

// bool Queue::isEmpty(){
//     if(front == -1 && rear == -1){
//         return true;
//     }
//     else {
//         return false;
//     }
// }

// bool Queue::isFull(){
//     if((rear + 1) % size == front){
//         return true;
//     }
//     else{
//         return false;
//     }
// }

// void Queue::enqueue(string data){
//     if(isFull()){
//         cout << "Queue is already full\n";
//         return;
//     }

//     if(isEmpty()){
//         front = rear = 0;
//     }
//     else {
//         rear = (rear + 1) % size;
//     }

//     queue[rear] = data;
// }

// void Queue::dequeue(){
//     if(isEmpty()){
//         cout << "Queue is already empty\n";
//         return;
//     }

//     else if (front == rear){
//         front = rear = -1;
//     }
//     else {
//         front = (front + 1) % size;
//     }
// }

// string Queue::peek(){
//     if(isEmpty()){
//         cout << "Queue is empty\n";
//         return "";
//     }

//     return queue[front];
// }

// void Queue::display(){
//     int count = (rear + size - front) % size + 1;

//     for(int i = 0; i < count;i++){
//         int index = (front + i) % size;

//         cout << queue[index] << endl;
//     }
// }
