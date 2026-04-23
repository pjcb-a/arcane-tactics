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

    //communicate
 
    printf(" %s\n\n", game.message);

        while (game.p1_status.hp > 0 && game.p2_status.hp > 0) {
            // Send game state to client
            
            if (send(client_sock, &game, sizeof(game), 0))
                die_with_error("Error: send() Failed.");
            
            printf("Your HP: %d, Your Energy: %d\n", game.p2_status.hp, game.p2_status.energy);
            printf("Opponent HP: %d, Opponent Energy: %d\n", game.p1_status.hp, game.p1_status.energy);


            printf(" \n---- YOUR HAND ---- \n\n");
            for(int i = 0; i < game.p1_status.hand_count; i++) {
            printf("[%d] %s (DMG:%d, UTIL:%d, COST:%d)\n", i,
                game.p1_status.hand[i].name,
                game.p1_status.hand[i].damage,
                game.p1_status.hand[i].utility,
                game.p1_status.hand[i].cost);
            }            
            
            //P1 card move
            printf("\n< Select card index to play >>  ");
            int p1_choice;
            scanf("%d", &p1_choice);

            //P2 card move (recieve from client)
            printf("\nWaiting for opponent's move...\n");
            int p2_choice;

            n = recv(client_sock, &p2_choice, sizeof(int), 0);
            if (n < 0){
                die_with_error("Error: Client Disconnected...\n");
                break;
            }

            printf("Log: P1 chose %d, P2 chose %d\n", p1_choice, p2_choice);
        game.p2_status.hp -= 10; 
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