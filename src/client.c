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
    
    int client_sock,  port_no, n;
    struct sockaddr_in server_addr;
    struct hostent *server;
    char buffer[32];
    int p1_choice, p2_choice;

    GameState game;

    if (argc < 3) {
        fprintf(stderr, "Usage: %s <hostname> <port>\n", argv[0]);
        exit(1);
    }
    //client initialization
    printf("Client starting ...\n");
    client_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (client_sock < 0)
        die_with_error("Error: socket() Failed.");

    //server connection setup
    printf("Looking for host '%s'...\n", argv[1]);
    server = gethostbyname(argv[1]);
    if (server == NULL)
        die_with_error("Error: No such host.");
    printf("Host found!\n");

    port_no = atoi(argv[2]);
    bzero((char *) &server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr_list[0], (char *)&server_addr.sin_addr.s_addr, server->h_length);
    server_addr.sin_port = htons(port_no);

    printf("Connecting to server at port %d...\n", port_no);
    if (connect(client_sock, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0)
        die_with_error("Error: connect() Failed.");

    printf("Connection successful!\n");





// Recieve game state of the dice roll from server
        n = recv(client_sock, &game, sizeof(game), 0);
        if(n < 0){
                die_with_error("Error: Server Disconnected...\n");
        }

    // 1. PREPARATION PHASE -- Dice roll to determine who goes first
    printf(game.message);
    printf("--- DICE ROLL RESULT ---");
    printf("You (P1) rolled: %d\n", game.p1_roll);
    printf("Opponent (P2) rolled: %d\n", game.p2_roll);
    printf("------------------------\n\n");

        if(game.p2_roll > game.p1_roll) {
            printf("Tou (P2) go first!\n");
        } else {
            printf("AIN'T NO WAAAYY, opponent goes first!\n");
        }

    // 2. GAME PHASE -- Display of cards and start of round 
        while(1) {
                n = recv(client_sock, &game, size_of(game), 0);
                if(n < 0){
                    die_with_error("Error: Server Disconnected...\n");
                    break;
                }

                printf("\n------------------------------------\n");
                printf( "%s", game.message);
                printf("\n------------------------------------\n");

                printf("Your HP: %d,   Your Energy: %d\n", game.p2_status.hp, game.p2_status.energy);
                printf("Opponent HP: %d,   Opponent Energy: %d\n", game.p1_status.hp, game.p1_status.energy);
        
                printf(" \n---- YOUR HAND ---- \n\n");
                for(int i = 0; i < game.p2_status.hand_count; i++) {
                    printf("[%d] %s (DMG:%d, UTIL:%d, COST:%d)\n", i,
                    game.p2_status.hand[i].name,
                    game.p2_status.hand[i].damage,
                    game.p2_status.hand[i].utility,
                    game.p2_status.hand[i].cost);
            }

            printf("\n< Select card index to play >> ");
            fgets(buffer, sizeof(buffer), stdin);
            p2_choice = buffer;
            send(client_sock, &p2_choice, sizeof(int), 0);
        
            printf("Waiting for opponent's move...\n");
            printf("Waiting for server to process turn...\n");

        }

    close(client_sock);
    return 0;
}