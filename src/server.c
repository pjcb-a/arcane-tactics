#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include "common.h"
#include <time.h>


int main(int argc, char *argv[]){

    srand(time(NULL)); // Seed the random number generator for card drawing

    int server_sock, client_sock, port_no, n;
    struct sockaddr_in server_addr, client_addr;

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
    
    draw_card(&game.p1_status, START_HAND_SIZE);
    draw_card(&game.p2_status, START_HAND_SIZE);

    //communicate
 
        while (game.p1_status.hp > 0 && game.p2_status.hp > 0) {
            // Send game state to client
            send(client_sock, &game, sizeof(game), 0);
            
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

                //p2 card move
                printf("Waiting for opponent's move...\n");
                int p2_choice;
                recv(client_sock, &p2_choice, sizeof(int), 0);
            }


            close(client_sock);
        close(server_sock);
        }
    return 0; 
}