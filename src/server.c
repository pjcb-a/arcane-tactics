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

    printf("Client successfully connected ...\n\n");

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

    strcpy(game.message, "Welcome to Arcane Tactics! Match Initiated.");

    draw_card(&game.p1_status, START_HAND_SIZE);
    draw_card(&game.p2_status, START_HAND_SIZE);

    // Dice roll to determine who goes first
    int winner = dice_roll(&game.p1_roll, &game.p2_roll);

    printf(" %s\n\n", game.message);
    printf("--- DICE ROLL RESULT ---\n");
    printf("You (P1) rolled: %d\n", game.p1_roll);
    printf("Opponent (P2) rolled: %d\n", game.p2_roll);

    if (winner == 1) {
        printf("You (P1) go first!\n");
    } else {
        printf("Ain't no way opponent goes first!\n"); // Opponent (P2) goes first!
    }
    printf("------------------------\n\n");

    // Send initial game state (including dice roll) to client
    send(client_sock, &game, sizeof(game), 0);

    while (game.p1_status.hp > 0 && game.p2_status.hp > 0) {

        for(int i = 1; i < game.p1_status.hand_count; i++) {
            printf("     . . . ROUND %d . . . \n      ---  START ! ---\n\n", i);
            
            printf("Your HP: %d, Your Energy: %d\n", game.p1_status.hp, game.p1_status.energy);
            printf("Opponent HP: %d, Opponent Energy: %d\n", game.p2_status.hp, game.p2_status.energy);

            printf(" \n---- YOUR HAND ---- \n\n");
            for(int j = 1; j <= game.p1_status.hand_count; j++) {
                printf("[%d] %s (DMG:%d, UTIL:%d, COST:%d)\n", j,
                    game.p1_status.hand[j].name,
                    game.p1_status.hand[j].damage,
                    game.p1_status.hand[j].utility,
                    game.p1_status.hand[j].cost);
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
                n = recv(client_sock, buffer, sizeof(int), 0);
                p2_choice = atoi(buffer);
                if (n < 0) {
                    die_with_error("Error: Client Disconnected...\n");
                    break;
                }

            } else {
                // P2 (client) goes first - receive their choice first
                printf("\nWaiting for opponent's move...\n");
                bzero(buffer, 32);
                n = recv(client_sock, buffer, sizeof(int), 0);
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

            printf("Round %d P1 chose %d, P2 chose %d\n", i, p1_choice, p2_choice);
            // TODO: Process cards based on priority -- added ni xian
                    // Get the cards played
                Card p1_card = Card_Pool[p1_choice - 1];  // -1 because hand is 1-indexed
                Card p2_card = Card_Pool[p2_choice - 1];

                // Determine turn order based on priority
                if (p1_card.priority > p2_card.priority) {
                    apply_card_effect(&game.p1_status, &game.p2_status, &p1_card);
                    apply_card_effect(&game.p2_status, &game.p1_status, &p2_card);
                } else {
                    apply_card_effect(&game.p2_status, &game.p1_status, &p2_card);
                    apply_card_effect(&game.p1_status, &game.p2_status, &p1_card);
                }

                // Update status effects at end of turn
                update_status_effects(&game.p1_status);
                update_status_effects(&game.p2_status);
            
            // Send updated game state to client at end of each turn
            send(client_sock, &game, sizeof(game), 0);
        }
    }

// Push to PQ: Add both choices to your Priority Queue.

// Sort/Process:

//     If Card_Pool[p1_choice].priority > Card_Pool[p2_choice].priority, execute P1's damage first.

//     Update the GameState HP values.

// Broadcast: Send the updated GameState back to the client.
   
close(client_sock);
    close(server_sock);
    return 0; 
}