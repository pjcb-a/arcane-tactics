#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include "common.h"


int main(int argc,  char *argv[]){
    
    int client_sock,  port_no,  n;
    struct sockaddr_in server_addr;
    struct hostent *server;
    
    GameState game;
    Player p1 = {MAX_HP, START_ENERGY, {0}, 0}; //Server
    Player p2 = {MAX_HP, START_ENERGY, {0}, 0}; //Client
    // Initialize game state for both players (server and client)
    game.p1_status.hp = MAX_HP;
    game.p1_status.energy = START_ENERGY;
    game.p1_status.hand_count = 0;

    game.p2_status.hp = MAX_HP;
    game.p2_status.energy = START_ENERGY;
    game.p2_status.hand_count = 0;

    strcpy(game.message, "Welcome to Arcane Tactics! Match Initiated.");

    while(1) {
        recv(client_sock, &game, sizeof(game), 0);

        printf("Message: %s\n", game.message);
        printf("Your HP: %d, Your Energy: %d\n", game.p2_status.hp, game.p2_status.energy);
        printf("Opponent HP: %d, Opponent Energy: %d\n", game.p1_status.hp, game.p1_status.energy);
    
        printf(" ---- YOUR HAND ---- \n");
            for(int i = 0; i < game.p1_status.hand_count; i++) {
                printf("[%d] %s\n", i, game.p1_status.hand[i].name);

                //p1 card move
                printf("< Select card index to play >>");
                int choice;
                scanf("%d", &choice);
                send(client_sock, &choice, sizeof(int), 0);

                //p2 card move
                printf("Waiting for opponent's move...\n");
                int p2_choice;
                recv(client_sock, &p2_choice, sizeof(int), 0);
            }
    }
    return 0;
}