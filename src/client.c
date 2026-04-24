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

    GameState game;

    strcpy(game.message, "Welcome to Arcane Tactics! Match Initiated.");

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

    printf("%s\n\n", game.message);

    // Wait for initial game state with dice roll result
    n = recv(client_sock, &game, sizeof(game), 0);
    if(n < 0){
        die_with_error("Error: Server Disconnected...\n");
        return 1;
    }

    // Display dice roll result on client side
    printf("=== DICE ROLL RESULT ===\n");
    printf("You (P2) rolled: %d\n", game.p2_roll);
    printf("Opponent (P1) rolled: %d\n", game.p1_roll);
    if (game.p1_roll > game.p2_roll) {
        printf("Ain't no way opponent goes first!\n"); //Opponent (P1) goes first!
    } else {
        printf("You (P2) go first!\n");
    }
    printf("========================\n\n");

        while(1) {

            printf("Your HP: %d, Your Energy: %d\n", game.p2_status.hp, game.p2_status.energy);
            printf("Opponent HP: %d, Opponent Energy: %d\n", game.p1_status.hp, game.p1_status.energy);

            printf(" \n---- YOUR HAND ---- \n\n");
                for(int i = 0; i < game.p2_status.hand_count; i++) {
                    printf("[%d] %s (DMG:%d, UTIL:%d, COST:%d)\n", i,
                    game.p2_status.hand[i].name,
                    game.p2_status.hand[i].damage,
                    game.p2_status.hand[i].utility,
                    game.p2_status.hand[i].cost);
            }

            //p2 card move
            printf("\n< Select card index to play >> ");
            int p2_choice;
            scanf("%d", &p2_choice);
            send(client_sock, &p2_choice, sizeof(int), 0);

            //opponents move
            printf("Waiting for opponent's move...\n");

            // Receive updated game state from server
            n = recv(client_sock, &game, sizeof(game), 0);
            if(n < 0){
                die_with_error("Error: Server Disconnected...\n");
                break;
            }
        }
        
    close(client_sock);
    return 0;
}