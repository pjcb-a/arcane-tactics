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

    int server_sock, client_sock, port_no, client_size, n;
    struct sockaddr_in server_addr, client_addr;

    // Avoid buffer overflows 
    char buffer[32];
    int p1_choice, p2_choice;
    int round_num = 1;
    
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <port>\n", argv[0]);
        exit(1);
    }

    // server initialization
    printf("Server starting ...\n");
    server_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (server_sock < 0)
        die_with_error("Error: socket() Failed.");

    bzero((char *) &server_addr, sizeof(server_addr));
    port_no = atoi(argv[1]);
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port_no);

    if (bind(server_sock, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0)
        die_with_error("Error: bind() Failed.");

    listen(server_sock, 5);
    printf("Server listening on port %d ...\n", port_no);

    client_size = sizeof(client_addr);
    client_sock = accept(server_sock, (struct sockaddr *) &client_addr, &client_size);
    if (client_sock < 0)
        die_with_error("Error: accept() Failed.");

    printf("Client successfully connected ...\n");

    GameState game;
    memset(&game, 0, sizeof(GameState));

    Player p1; //Server
    Player p2; //Client
    // Initialize game state for both players (server and client)
    game.p1_status.hp = MAX_HP;
    game.p1_status.energy = START_ENERGY;
    game.p1_status.hand_count = 0;

    game.p2_status.hp = MAX_HP;
    game.p2_status.energy = START_ENERGY;
    game.p2_status.hand_count = 0;


    // 1. PREPARATION PHASE -- Dice roll to determine who goes first
    int winner = dice_roll(&game.p1_roll, &game.p2_roll);

    sprintf(game.message, "Welcome to Arcane Tactics! Match Initiated.");

    printf("\n%s\n", game.message);
    printf("\n--- DICE ROLL RESULT ---\n");
    printf("You (P1) rolled: %d\n", game.p1_roll);
    printf("Opponent (P2) rolled: %d\n", game.p2_roll);
    printf("------------------------\n\n");

    // Send dice roll status to client
    send(client_sock, &game, sizeof(game), 0);

    if (winner == 1) {
        printf("You (P1) go first!\n");
    } else {
        printf("AIN'T NO WAAAYY, opponent goes first!\n"); 
    }


    draw_card(&game.p1_status, START_HAND_SIZE);
    draw_card(&game.p2_status, START_HAND_SIZE);

    // 2. GAME PHASE -- Actual loop of the game, the server is both the player and the "game master" !! REMEMBER !! 
    while (game.p1_status.hp > 0 && game.p2_status.hp > 0) {
            printf("\n------------------------------------\n");
            printf( "  - - - ROUND %d - - - START! - - -  ", round_num);
            printf("\n------------------------------------\n");

            //Send the round also to client
            sprintf(game.message, "  - - - ROUND %d - - - START! - - -  ", round_num);
            send(client_sock, &game, sizeof(game), 0);

            printf("Your HP: %d,   Your Energy: %d\n", game.p1_status.hp, game.p1_status.energy);
            printf("Opponent HP: %d,   Opponent Energy: %d\n", game.p2_status.hp, game.p2_status.energy);

            printf(" \n---- YOUR HAND ---- \n\n");
            int j = 1;
            for(int i = 0; i < game.p1_status.hand_count; i++) {
                printf("[%d] %s (DMG:%d, UTIL:%d, COST:%d)\n", j,
                    game.p1_status.hand[i].name,
                    game.p1_status.hand[i].damage,
                    game.p1_status.hand[i].utility,
                    game.p1_status.hand[i].cost);
                    j++;
            }

            // Dice winner goes first
            if (winner == 1) {
                // P1 (server) goes first
                printf("\n< Select card index to play >>  ");
                fgets(buffer, sizeof(buffer), stdin);
                p1_choice = atoi(buffer);

                // Receive P2 choice
                printf("\nWaiting for opponent's move...\n");
                bzero(buffer, 32);
                n = recv(client_sock, buffer, sizeof(buffer), 0);
                p2_choice = atoi(buffer);
                if (n < 0) {
                    die_with_error("Error: Client Disconnected...\n");
                    break;
                }

            } else {
                // P2 (client) goes first - receive their choice first
                printf("\nWaiting for opponent's move...\n");
                bzero(buffer, 32);
                n = recv(client_sock, buffer, sizeof(buffer), 0);
                p2_choice = atoi(buffer);
                if (n < 0) {
                    die_with_error("Error: Client Disconnected...\n");
                    break;
                }

                // Now P1 makes choice
                printf("\n< Select card index to play >>  ");
                fgets(buffer, sizeof(buffer), stdin);
                p1_choice = atoi(buffer);
            }

            // 3. CLOSURE PHASE -- (aray mo) The round results and prep for another round.  
            sprintf(game.message, "Round %d CARDS DRAWN: P1 chose [%s], P2 chose [%s]", 
                round_num, 
                game.p1_status.hand->name, 
                game.p2_status.hand->name);

            printf("%s\n", game.message);
            
            //PRIORITY QUEUE OF CARDS HERE
            
            // Declaration of queues
            ActionQueue firstRoll;
            ActionQueue secondRoll;
            init_queue(&firstRoll, 10);
            init_queue(&secondRoll, 10);

            Action p1_action = {1, game.p1_status.hand[p1_choice]}; 
            Action p2_action = {2, game.p2_status.hand[p2_choice]}; 

            // PLACE ENQUEUE OF P1 and P2 HERE !!
            // Then after combat phase ends, dequeue the used card and draw 1 card.

            while( game.p1_status.energy > 0 || game.p2_status.energy > 0 || game.p1_status.energy < game.p1_status.hand_count || game.p2_status.energy < game.p2_status.hand_count ) {
                
                if(game.p1_status.hand->priority == 0 || game.p2_status.hand->priority == 0){
                    enqueue(&firstRoll, p1_action);
                    enqueue(&firstRoll, p2_action);
                }
                else {
                    enqueue(&secondRoll, p1_action);
                    enqueue(&secondRoll, p2_action);
                }
            }

            game.p1_status.hand_count--;
            game.p2_status.hand_count--;
            draw_card(&game.p1_status, 1);
            draw_card(&game.p2_status, 1);
            round_num++;

        }
    

    // TO-DO
    // 1. fix game logic for the priority card attack   -- ?  function : constraint
    // 2. shield logic -- medj done
    // 3. send update of card used to client   -- fix send and recv sockets - ONGOING
    // 4. Card redraw logic when a player has used a card  -- fix card increment/decrement 
    // 5. fix card effects logic -- connect and -- ? logic : function
   
close(client_sock);
    close(server_sock);
    return 0; 
}